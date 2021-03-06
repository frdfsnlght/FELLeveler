#ifndef SOCKIO_H
#define SOCKIO_H

#include <WebSocketsServer.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>
#include <map>

class SockIO {

    private:

    enum MessageType {
        Ping = 2,
        Pong = 3,
        Message = 4,
        Ack = 5
    };

    struct MessageStruct {
        MessageType type;
        unsigned int ackId;
        ArduinoJson::JsonArray* array;
        ArduinoJson::JsonDocument* doc;
        MessageStruct(MessageType type, unsigned int ackId, ArduinoJson::JsonArray* array):
            type(type),
            ackId(ackId),
            array(array) {}
        MessageStruct(ArduinoJson::JsonDocument* doc): doc(doc) {}
    };

    static bool serializeMessage(MessageStruct& msg, String& buffer);
    static bool deserializeMessage(const String& buffer, MessageStruct& msg);

    //static String formatMessage(SockIO::MessageType msgType, unsigned int ackId, JsonArray& args);

    friend class SockIOServer;
    friend class SockIOServerClient;
    friend class SockIOClient;

};

class SockIOServerClient;

class SockIOServer {

    public:

    SockIOServer(
        uint16_t port,
        const String& origin,
        uint32_t pingInterval = 200,
        uint32_t pongTimeout = 2000,
        uint8_t disconnectTimeoutCount = 4);
    ~SockIOServer();

    void begin();
    void end();
    
    void loop();

    void on(String event, void (*cb)(SockIOServerClient&));
    void emit(String message);
    void emit(String message, JsonArray& args);

    private:

    WebSocketsServer* socket;
    std::map<String, void (*)(SockIOServerClient&)> handlers;
    std::map<uint8_t, SockIOServerClient*> clients;
    unsigned int nextRequestId = 1;

    void send(int clientNum, SockIO::MessageType msgType, unsigned int ackId, JsonArray& args);
    void triggerEvent(const String& event, SockIOServerClient& client);

    friend class SockIOServerClient;

};

class SockIOServerClient {

    public:

    void on(String event, void (*cb)(SockIOServerClient& client,JsonArray& args,JsonArray& ret));
    void emit(String message, void(*cb)(JsonArray&) = NULL);
    void emit(String message, JsonArray& args, void(*cb)(JsonArray&) = NULL);

    private:

    SockIOServerClient(SockIOServer& server, uint8_t client): server(server), client(client) {}
    ~SockIOServerClient();

    SockIOServer& server;
    uint8_t client;
    std::map<String, void (*)(SockIOServerClient&,JsonArray&,JsonArray&)> handlers;
    std::map<unsigned int, void(*)(JsonArray&)> requests;
    unsigned int nextRequestId = 1;

    void triggerEvent(const String& event);
    void processData(const String& data);

    friend class SockIOServer;

};

class SockIOClient {

    public:

    SockIOClient(
        const String& host,
        uint16_t port,
        const String& origin);
    ~SockIOClient();

    void begin();
    void end();
    
    void loop();

    void on(String event, void (*cb)(JsonArray& args,JsonArray& ret));
    void emit(String message, void(*cb)(JsonArray&) = NULL);
    void emit(String message, JsonArray& args, void(*cb)(JsonArray&) = NULL);

    private:

    String host;
    uint16_t port;
    String origin;
    WebSocketsClient* socket;
    std::map<String, void (*)(JsonArray&,JsonArray&)> handlers;
    std::map<unsigned int, void(*)(JsonArray&)> requests;
    unsigned int nextRequestId = 1;

    void send(SockIO::MessageType msgType, unsigned int ackId, JsonArray& args);
    void triggerEvent(const String& event);
    void processData(const String& data);

};

#endif