#include "sockio.h"

#include "debug.h"

#ifdef DEBUG_SOCKIO
#define DEBUG(...) Serial.printf(__VA_ARGS__)
#else
#define DEBUG(...)
#endif

SockIOServer::SockIOServer(
    uint16_t port,
    const String& origin,
    uint32_t pingInterval = 200,
    uint32_t pongTimeout = 2000,
    uint8_t disconnectTimeoutCount = 5) {
    socket = new WebSocketsServer(port, origin, "SockIO");
    socket->enableHeartbeat(pingInterval, pongTimeout, disconnectTimeoutCount);
    SockIOServer* self = this;
    socket->onEvent([self](uint8_t clientNum, WStype_t type, uint8_t* payload, size_t length) {
        SockIOServerClient* client = self->clients[clientNum];
        switch (type) {
            case WStype_CONNECTED:
                DEBUG("SockIO client %d connected\n", clientNum);
                client = new SockIOServerClient(*self, clientNum);
                self->clients[clientNum] = client;
                self->triggerEvent("connected", *client);
                client->triggerEvent("connected");
                break;
            case WStype_DISCONNECTED:
                DEBUG("SockIO client %d disconnected\n", clientNum);
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
                DEBUG("SockIO received invalid frame from client %d\n", clientNum);
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
    JsonArray array = doc.as<JsonArray>();
    array.add(message);
    send(-1, SockIO::Message, 0, array);
}

void SockIOServer::emit(String message, JsonArray& args) {
    StaticJsonDocument<1024> doc;
    JsonArray array = doc.as<JsonArray>();
    array.add(message);
    array.add(args);
    send(-1, SockIO::Message, 0, array);
}


void SockIOServer::send(int clientNum, SockIO::MessageType msgType, unsigned int ackId, JsonArray& array) {
    int len = 1;
    if (ackId) len += 8;
    len += measureJson(array);
    String buffer;
    buffer.reserve(len);
    buffer += msgType;
    if (ackId) buffer += ackId;
    buffer += '|';
    serializeJson(array, buffer);
    if (clientNum != -1) {
        DEBUG("SockIO >%d %s\n", clientNum, buffer.c_str());
        socket->sendTXT(clientNum, buffer);
    } else {
        DEBUG("SockIO >* %s\n", buffer.c_str());
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

void SockIOServerClient::emit(String message) {
    StaticJsonDocument<128> doc;
    JsonArray array = doc.as<JsonArray>();
    array.add(message);
    server.send(client, SockIO::Message, 0, array);
}

void SockIOServerClient::emit(String message, JsonArray& args) {
    StaticJsonDocument<1024> doc;
    JsonArray array = doc.as<JsonArray>();
    array.add(message);
    array.add(args);
    server.send(client, SockIO::Message, 0, array);
}

void SockIOServerClient::emit(String message, void(*cb)(JsonArray&)) {
    StaticJsonDocument<128> doc;
    JsonArray array = doc.as<JsonArray>();
    array.add(message);
    unsigned int ackId = nextRequestId++;
    requests[ackId] = cb;
    server.send(client, SockIO::Message, ackId, array);
}

void SockIOServerClient::emit(String message, JsonArray& args, void(*cb)(JsonArray&)) {
    StaticJsonDocument<1024> doc;
    JsonArray array = doc.as<JsonArray>();
    array.add(message);
    array.add(args);
    unsigned int ackId = nextRequestId++;
    requests[ackId] = cb;
    server.send(client, SockIO::Message, ackId, array);
}

void SockIOServerClient::triggerEvent(const String& event) {
    void (*handler)(SockIOServerClient&,JsonArray&,JsonArray&) = handlers[event];
    if (handler)
        handler(*this,*(JsonArray*)0, *(JsonArray*)0);
}

void SockIOServerClient::processData(const String& data) {
    DEBUG("SockIO <%d %s\n", client, data.c_str());
    void (*handler)(SockIOServerClient&,JsonArray&,JsonArray&);
    void(*requester)(JsonArray&);
    String msg;
    StaticJsonDocument<1024> inDoc;
    StaticJsonDocument<1024> outDoc;
    SockIO::MessageType msgType = (SockIO::MessageType)(data[0] - '0');
    unsigned int ackId = 0;
    String json;
    int sepPos = data.indexOf('|');
    if (sepPos != -1) {
        ackId = data.substring(1, sepPos).toInt();
        json = data.substring(sepPos + 1);
    } else
        json = data.substring(1);
    DeserializationError err = deserializeJson(inDoc, json);
    if (err) {
        Serial.printf("SockIO JSON serialization error: %s\n", err.c_str());
        return;
    }
    if (! inDoc.is<JsonArray>()) {
        Serial.printf("SockIO JSON is not an array: %s\n", data.c_str());
    }
    JsonArray inArray = inDoc.as<JsonArray>();
    JsonArray outArray = outDoc.to<JsonArray>();

    switch (msgType) {
        case SockIO::Ping:
            server.send(client, SockIO::Pong, 0, inArray);
            return;
        case SockIO::Pong:
            // NOP, we don't send pings from the server
            return;
        case SockIO::Message:
            if (inArray.size() == 0) {
                Serial.printf("SockIO too few arguments for message type message %s\n", data.c_str());
                return;
            }
            msg = inArray[0].as<String>();
            inArray.remove(0);
            handler = handlers[msg];
            if (handler) {
                handler(*this,inArray, outArray);
                if (ackId) {
                    server.send(client, SockIO::Ack, ackId, outArray);
                }
            } else if (ackId) {
                Serial.printf("SockIO no handler found for \"%s\", sending empty ACK", msg.c_str());
                server.send(client, SockIO::Ack, ackId, outArray);
            }
            break;
        case SockIO::Ack:
            if (! ackId) {
                Serial.printf("SockIO expected ACK id but did not find one %s\n", data.c_str());
                return;
            }
            requester = requests[ackId];
            if (! requester) {
                Serial.printf("SockIO no requestor for ACK id %s\n", data.c_str());
                return;
            }
            requester(inArray);
            break;
        default:
            Serial.printf("SockIO received unknown message type %d\n", msgType);
            return;
    }

}
