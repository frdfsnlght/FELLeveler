
export class SockIOClient {

    public requestsInFlight: number = 0;
    public pingTime: number = 0;

    private url: string;
    private pingInterval: number = 0;
    private pongTimeout: number = 0;
    private disconnectTimeoutCount: number = 0;
    private reconnectInterval: number = 2000;

    private nextRequestId: number = 1;
    private wasOpen: boolean = false;

    private handlers = new Map<string, (...args: any)=>void>();
    private requests = new Map<number, (...args: any)=>void>();
    private messageQueue = new Array<string>();

    private missedPongs: number = 0;

    private socket: WebSocket | undefined;
    private pingTimer: ReturnType<typeof setInterval> | undefined;
    private pongTimer: ReturnType<typeof setTimeout> | undefined;
    private reconnectTimer: ReturnType<typeof setInterval> | undefined;

//    private const TYPE_CONNECT: number = 0;
//    private const TYPE_DISCONNECT: number = 1;
    private static readonly TYPE_PING: number = 2;
    private static readonly TYPE_PONG: number = 3;
    private static readonly TYPE_MESSAGE: number = 4;
    private static readonly TYPE_ACK: number = 5;

    constructor(url: string, options?: {
        pingInterval?: number,
        pongTimeout?: number,
        disconnectTimeoutCount?: number}) {
        this.url = url;
        this.pingInterval = options?.pingInterval === undefined ? 0 : options.pingInterval;
        this.pongTimeout = options?.pongTimeout === undefined ? 0 : options.pongTimeout;
        this.disconnectTimeoutCount = options?.disconnectTimeoutCount === undefined ? 0 : options.disconnectTimeoutCount;
        this.setupReconnectTimer();
    }
    
    connected(): boolean {
        return this.socket != undefined && this.socket.readyState == WebSocket.OPEN;
    }

    on(event: string, cb:(...args: any)=>void): void {
        this.handlers.set(event, cb);
    }

    emit(message: string, ...args: any): void {
        let cb = undefined;
        if (args.length && typeof args[args.length - 1] === 'function') {
            cb = args.pop();
        }
        args.unshift(message);
        //console.debug(args);
        this.send(SockIOClient.TYPE_MESSAGE, args, cb);
    }

    private send(msgType: number, args: any, cb?: (...args: any)=>(void)): void {
        let out: string = msgType.toString();
        if (cb != undefined) {
            let id = this.nextRequestId++;
            out += id.toString();
            this.requests.set(id, cb);
            this.requestsInFlight = this.requests.size;
        }
        out += '|';
        out += JSON.stringify(args);
        if (this.connected()) {
            console.debug('SockIO >', out);
            this.socket?.send(out);
        } else {
            this.messageQueue.push(out);
            console.log('SockIO Q', out);
        }
    }

    private connect(): void {
        if (this.socket) {
            let state = '';
            switch (this.socket.readyState) {
                case WebSocket.CLOSING:
                    console.log('SockIO waiting for close');
                    return;
                case WebSocket.OPEN:
                    console.log('SockIO socket is already open!');
                    return;
                case WebSocket.CONNECTING:
                    console.log('SockIO waiting for connection');
                    return;
                case WebSocket.CLOSED:
                    break;
            }
            this.socket.onopen = null;
            this.socket.onerror = null;
            this.socket.onclose = null;
            this.socket = undefined;
        }
        console.log('SockIO starting connection');
        let ws = new WebSocket(this.url);
        ws.onopen = () => {
            console.log('SockIO client connected');
            this.wasOpen = true;
            this.clearReconnectTimer();
            while (this.messageQueue.length) {
                let out = this.messageQueue.shift();
                if (out != undefined) {
                    console.log('SockIO Q>', out);
                    ws.send(out);
                }
            }
            this.triggerEvent('connected');
            this.setupPingTimer();
            this.setupPongTimer();
        };
        
        ws.onclose = () => {
            if (this.wasOpen) {
                this.wasOpen = false;
                console.log('SockIO closed');
                this.triggerEvent('disconnected');
                this.clearPongTimer();
                this.clearPingTimer();
            }
            this.setupReconnectTimer();
        };

        ws.onerror = (err) => {
            if (this.wasOpen) {
                console.log('SockIO error', err);
                ws.close();
            }
        };

        ws.onmessage = (e) => {
            console.debug('SockIO <', e.data);
            let inMsg: string = e.data;
            let msgType: number = parseInt(inMsg.substring(0, 1));
            let ackId: number | undefined = undefined;
            inMsg = inMsg.slice(1);
            let sepPos: number = inMsg.indexOf('|');
            if (sepPos != -1) {
                ackId = parseInt(inMsg.substring(0, sepPos));
                inMsg = inMsg.slice(sepPos + 1);
            }
            let args: any = JSON.parse(inMsg);

            switch (msgType) {
                case SockIOClient.TYPE_PING:
                    this.send(SockIOClient.TYPE_PONG, args, undefined);
                    return;
                case SockIOClient.TYPE_PONG:
                    this.setupPongTimer();
                    this.missedPongs = 0;
                    this.pingTime = Date.now() - args[0];
                    console.debug('SockIO rtt:', this.pingTime, 'ms');
                    return;
                case SockIOClient.TYPE_MESSAGE:
                    if (args.length == 0) {
                        console.log('SockIO too few arguments for message type message', e.data);
                        return;
                    }
                    let event: string = args.shift().toString();
                    this.triggerEvent(event, ...args);
                    break;
                case SockIOClient.TYPE_ACK:
                    if (ackId == undefined) {
                        console.log('SockIO expected ACK id but did not find one', e.data);
                        return;
                    }
                    let requester = this.requests.get(ackId);
                    if (requester == undefined) {
                        console.log('SockIO no requestor for ACK id', e.data);
                        return;
                    }
                    this.requests.delete(ackId);
                    requester(...args);
                    this.requestsInFlight = this.requests.size;
                    break;
                default:
                    console.log('SockIO received unknown message type', e.data);
                    return;
            }
    
        };
        
        this.socket = ws;
    }

    private triggerEvent(event: string, ...args: any): void {
        let handler = this.handlers.get(event);
        if (handler) {
            //console.debug('found matching event handler for "' + event + '"');
            handler(...args);
        }
    }

    private sendPing(): void {
        if (! this.connected()) return;
        this.send(SockIOClient.TYPE_PING, [Date.now()]);
    }

    private setupReconnectTimer(): void {
        this.clearReconnectTimer();
        this.reconnectTimer = setInterval(() => { this.connect(); }, this.reconnectInterval);
        //console.debug('setup reconnect');
    }
    
    private clearReconnectTimer(): void {
        if (this.reconnectTimer) {
            clearInterval(this.reconnectTimer);
            this.reconnectTimer = undefined;
            //console.debug('cleared reconnect');
        }
    }
    
    private setupPingTimer(): void {
        this.clearPingTimer();
        if (this.pingInterval > 0) {
            this.pingTimer = setInterval(() => { this.sendPing(); }, this.pingInterval);
            //console.debug('setup ping');
        }
    }
    
    private clearPingTimer(): void {
        if (this.pingTimer) {
            clearInterval(this.pingTimer);
            this.pingTimer = undefined;
            //console.debug('cleared ping');
        }
    }
    
    private setupPongTimer(): void {
        this.clearPongTimer();
        if (this.pingInterval > 0) {
            this.pongTimer = setTimeout(() => { this.pongMissed(); }, this.pongTimeout);
            //console.debug('setup pong');
        }
    }

    private clearPongTimer(): void {
        if (this.pongTimer) {
            clearTimeout(this.pongTimer);
            this.pongTimer = undefined;
            //console.debug('cleared pong');
        }
    }

    private pongMissed(): void {
        this.missedPongs++;
        if (this.missedPongs >= this.disconnectTimeoutCount) {
            console.log('SockIO exceeded missed pongs');
            this.clearPingTimer();
            this.clearPongTimer();
            this.setupReconnectTimer();
            this.socket?.close();
        }
    }

}
