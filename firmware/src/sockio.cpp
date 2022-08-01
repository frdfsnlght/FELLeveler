#include "sockio.h"

#include <Arduino.h>

bool SockIO::serializeMessage(MessageStruct& msg, String& buffer) {
    int len = 1;
    if (msg.ackId) len += 8;
    len += measureJson(*msg.array);
    if (! buffer.reserve(len)) {
        log_w("SockIO unable to reserve buffer of size %d!", len);
        return false;
    }
    buffer.clear();
    buffer += msg.type;
    if (msg.ackId) buffer += msg.ackId;
    buffer += '|';
    serializeJson(*msg.array, buffer);
    return true;
}

bool SockIO::deserializeMessage(const String& buffer, MessageStruct& msg) {
    msg.type = (SockIO::MessageType)(buffer[0] - '0');
    msg.ackId = 0;
    String json;
    int sepPos = buffer.indexOf('|');
    if (sepPos != -1) {
        msg.ackId = buffer.substring(1, sepPos).toInt();
        json = buffer.substring(sepPos + 1);
    } else
        json = buffer.substring(1);
    DeserializationError err = deserializeJson(*msg.doc, json);
    if (err) {
        log_w("SockIO JSON deserialization error: %s", err.c_str());
        return false;
    }
    if (! msg.doc->is<JsonArray>()) {
        log_w("SockIO JSON is not an array: %s", buffer.c_str());
    }
    return true;
}

SockIOServer::SockIOServer(
    uint16_t port,
    const String& origin,
    uint32_t pingInterval,
    uint32_t pongTimeout,
    uint8_t disconnectTimeoutCount) {
    socket = new WebSocketsServer(port, origin, "SockIO");
    socket->enableHeartbeat(pingInterval, pongTimeout, disconnectTimeoutCount);
    SockIOServer* self = this;
    socket->onEvent([self](uint8_t clientNum, WStype_t type, uint8_t* payload, size_t length) {
        SockIOServerClient* client = self->clients[clientNum];
        switch (type) {
            case WStype_CONNECTED:
                log_d("SockIO client %d connected", clientNum);
                client = new SockIOServerClient(*self, clientNum);
                self->clients[clientNum] = client;
                self->triggerEvent("connected", *client);
                client->triggerEvent("connected");
                break;
            case WStype_DISCONNECTED:
                log_d("SockIO client %d disconnected", clientNum);
                if (client) {
                    client->triggerEvent("disconnected");
                    self->clients.erase(clientNum);
                    self->triggerEvent("disconnected", *client);
                    delete client;
                }
                break;
            case WStype_TEXT:
                if (client) {
                    String data = String((const char*)payload);
                    client->processData(data);
                }
                break;
            case WStype_BIN:
    		case WStype_FRAGMENT_TEXT_START:
	    	case WStype_FRAGMENT_BIN_START:
    		case WStype_FRAGMENT:
	    	case WStype_FRAGMENT_FIN:
                log_w("SockIO received invalid frame from client %d", clientNum);
                break;
        }
    });
}

SockIOServer::~SockIOServer() {
    handlers.clear();
    for (auto const& x : clients) {
        delete x.second;
    }
    clients.clear();
    delete socket;
}

void SockIOServer::begin() {
    socket->begin();
}

void SockIOServer::end() {
    for (auto const& x : clients) {
        x.second->triggerEvent("disconnected");
        triggerEvent("disconnected", *x.second);
        delete x.second;
    }
    socket->close();
}

void SockIOServer::loop() {
    socket->loop();
}

void SockIOServer::on(String event, void (*cb)(SockIOServerClient&)) {
    handlers[event] = cb;
}

void SockIOServer::emit(String message) {
    StaticJsonDocument<128> doc;
    JsonArray array = doc.to<JsonArray>();
    array.add(message);
    send(-1, SockIO::Message, 0, array);
}

void SockIOServer::emit(String message, JsonArray& args) {
    StaticJsonDocument<1024> doc;
    JsonArray array = doc.to<JsonArray>();
    array.add(message);
    for (JsonVariant value : args)
        array.add(value);
    send(-1, SockIO::Message, 0, array);
}

void SockIOServer::send(int clientNum, SockIO::MessageType msgType, unsigned int ackId, JsonArray& array) {
    SockIO::MessageStruct msg(msgType, ackId, &array);
    String buffer;
    if (! SockIO::serializeMessage(msg, buffer)) return;
    if (clientNum != -1) {
        log_d("SockIO >%d %s", clientNum, buffer.c_str());
        socket->sendTXT(clientNum, buffer);
    } else {
        log_d("SockIO >* %s", buffer.c_str());
        socket->broadcastTXT(buffer);
    }
}

void SockIOServer::triggerEvent(const String& event, SockIOServerClient& client) {
    void (*handler)(SockIOServerClient&) = handlers[event];
    if (handler)
        handler(client);
}


// -----------------------------------------------
// ServerClient

SockIOServerClient::~SockIOServerClient() {
    handlers.clear();
}

void SockIOServerClient::on(String event, void (*cb)(SockIOServerClient& client,JsonArray& args,JsonArray& ret)) {
    handlers[event] = cb;
}

void SockIOServerClient::emit(String message, void(*cb)(JsonArray&)) {
    StaticJsonDocument<128> doc;
    JsonArray array = doc.to<JsonArray>();
    array.add(message);
    unsigned int ackId = 0;
    if (cb) {
        ackId = nextRequestId++;
        requests[ackId] = cb;
    }
    server.send(client, SockIO::Message, ackId, array);
}

void SockIOServerClient::emit(String message, JsonArray& args, void(*cb)(JsonArray&)) {
    StaticJsonDocument<1024> doc;
    JsonArray array = doc.to<JsonArray>();
    array.add(message);
    for (JsonVariant value : args)
        array.add(value);
    unsigned int ackId = 0;
    if (cb) {
        ackId = nextRequestId++;
        requests[ackId] = cb;
    }
    server.send(client, SockIO::Message, ackId, array);
}

void SockIOServerClient::triggerEvent(const String& event) {
    void (*handler)(SockIOServerClient&,JsonArray&,JsonArray&) = handlers[event];
    if (handler)
        handler(*this,*(JsonArray*)0, *(JsonArray*)0);
}

void SockIOServerClient::processData(const String& data) {
    log_d("SockIO <%d %s", client, data.c_str());
    void (*handler)(SockIOServerClient&,JsonArray&,JsonArray&);
    void(*requester)(JsonArray&);
    String event;
    StaticJsonDocument<1024> inDoc;
    StaticJsonDocument<1024> outDoc;
    SockIO::MessageStruct msg(&inDoc);
    if (! SockIO::deserializeMessage(data, msg)) return;
    JsonArray inArray = inDoc.as<JsonArray>();
    JsonArray outArray = outDoc.to<JsonArray>();

    switch (msg.type) {
        case SockIO::Ping:
            server.send(client, SockIO::Pong, 0, inArray);
            return;
        case SockIO::Pong:
            // NOP, we don't send pings from the server
            return;
        case SockIO::Message:
            if (inArray.size() == 0) {
                log_e("SockIO too few arguments for message type message %s", data.c_str());
                return;
            }
            event = inArray[0].as<String>();
            inArray.remove(0);
            handler = handlers[event];
            if (handler) {
                handler(*this,inArray, outArray);
                if (msg.ackId) {
                    server.send(client, SockIO::Ack, msg.ackId, outArray);
                }
            } else if (msg.ackId) {
                log_w("SockIO no handler found for \"%s\", sending empty ACK", event.c_str());
                server.send(client, SockIO::Ack, msg.ackId, outArray);
            }
            break;
        case SockIO::Ack:
            if (! msg.ackId) {
                log_w("SockIO expected ACK id but did not find one %s", data.c_str());
                return;
            }
            requester = requests[msg.ackId];
            if (! requester) {
                log_w("SockIO no requestor for ACK id %s", data.c_str());
                return;
            }
            requests.erase(msg.ackId);
            requester(inArray);
            break;
        default:
            log_w("SockIO received unknown message type %d", msg.type);
            return;
    }

}

// -----------------------------------------------
// Client

SockIOClient::SockIOClient(
    const String& host,
    uint16_t port,
    const String& origin): host(host), port(port), origin(origin) {
    socket = new WebSocketsClient();
    SockIOClient* self = this;
    socket->onEvent([self](WStype_t type, uint8_t* payload, size_t length) {
        switch (type) {
            case WStype_CONNECTED:
                log_d("SockIO client connected");
                self->triggerEvent("connected");
                break;
            case WStype_DISCONNECTED:
                log_d("SockIO client disconnected");
                self->triggerEvent("disconnected");
                break;
            case WStype_TEXT: {
                String data = String((const char*)payload);
                self->processData(data);
                break;
            }
            case WStype_BIN:
    		case WStype_FRAGMENT_TEXT_START:
	    	case WStype_FRAGMENT_BIN_START:
    		case WStype_FRAGMENT:
	    	case WStype_FRAGMENT_FIN:
                log_w("SockIO received unsupported frame from server");
                break;
        }
    });
}

SockIOClient::~SockIOClient() {
    handlers.clear();
    delete socket;
}

void SockIOClient::begin() {
    socket->begin(host, port, origin, "SockIO");
}

void SockIOClient::end() {
    socket->disconnect();
}

void SockIOClient::loop() {
    socket->loop();
}

void SockIOClient::on(String event, void (*cb)(JsonArray& args,JsonArray& ret)) {
    handlers[event] = cb;
}

void SockIOClient::emit(String message, void(*cb)(JsonArray&)) {
    StaticJsonDocument<128> doc;
    JsonArray array = doc.to<JsonArray>();
    array.add(message);
    unsigned int ackId = 0;
    if (cb) {
        ackId = nextRequestId++;
        requests[ackId] = cb;
    }
    send(SockIO::Message, ackId, array);
}

void SockIOClient::emit(String message, JsonArray& args, void(*cb)(JsonArray&)) {
    StaticJsonDocument<1024> doc;
    JsonArray array = doc.to<JsonArray>();
    array.add(message);
    for (JsonVariant value : args)
        array.add(value);
    unsigned int ackId = 0;
    if (cb) {
        ackId = nextRequestId++;
        requests[ackId] = cb;
    }
    send(SockIO::Message, ackId, array);
}

void SockIOClient::send(SockIO::MessageType msgType, unsigned int ackId, JsonArray& array) {
    SockIO::MessageStruct msg(msgType, ackId, &array);
    String buffer;
    if (! SockIO::serializeMessage(msg, buffer)) return;
    log_d("SockIO > %s", buffer.c_str());
    socket->sendTXT(buffer);
}

void SockIOClient::triggerEvent(const String& event) {
    void (*handler)(JsonArray&,JsonArray&) = handlers[event];
    if (handler)
        handler(*(JsonArray*)0, *(JsonArray*)0);
}

void SockIOClient::processData(const String& data) {
    log_d("SockIO < %s", data.c_str());
    void (*handler)(JsonArray&,JsonArray&);
    void(*requester)(JsonArray&);
    String event;
    StaticJsonDocument<1024> inDoc;
    StaticJsonDocument<1024> outDoc;
    SockIO::MessageStruct msg(&inDoc);
    if (! SockIO::deserializeMessage(data, msg)) return;
    JsonArray inArray = inDoc.as<JsonArray>();
    JsonArray outArray = outDoc.to<JsonArray>();

    switch (msg.type) {
        case SockIO::Ping:
            // NOP, we don't expect pings from the server
            return;
        case SockIO::Pong:
            // NOP, we don't send pings from the client
            return;
        case SockIO::Message:
            if (inArray.size() == 0) {
                log_e("SockIO too few arguments for message type message %s", data.c_str());
                return;
            }
            event = inArray[0].as<String>();
            inArray.remove(0);
            handler = handlers[event];
            if (handler) {
                handler(inArray, outArray);
                if (msg.ackId) {
                    send(SockIO::Ack, msg.ackId, outArray);
                }
            } else if (msg.ackId) {
                log_w("SockIO no handler found for \"%s\", sending empty ACK", event.c_str());
                send(SockIO::Ack, msg.ackId, outArray);
            }
            break;
        case SockIO::Ack:
            if (! msg.ackId) {
                log_w("SockIO expected ACK id but did not find one %s", data.c_str());
                return;
            }
            requester = requests[msg.ackId];
            if (! requester) {
                log_w("SockIO no requestor for ACK id %s", data.c_str());
                return;
            }
            requests.erase(msg.ackId);
            requester(inArray);
            break;
        default:
            log_w("SockIO received unknown message type %d", msg.type);
            return;
    }
}
