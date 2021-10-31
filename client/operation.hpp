#include <vector>
#include <string>
#include "request.hpp"
#include "response.hpp"
#include "message.hpp"
#include "client.hpp"
#include "common.hpp"


#ifndef _OPERATION_H
#define _OPERATION_H

/* Supported Operation  */
#define OP_REGISTER (10)
#define OP_CLIENT_LIST (20)
#define OP_PUBLIC_KEY_REQUEST (30)
#define OP_FETCH_MESSAGES (40)
#define OP_SEND_MESSAGE (50)
#define OP_GET_SYM_KEY (51)
#define OP_SEND_SYM_KEY (52)

#define SUPPORTED_OPERATIONS {OP_REGISTER, OP_CLIENT_LIST, \
                            OP_PUBLIC_KEY_REQUEST,OP_FETCH_MESSAGES, \
                            OP_SEND_MESSAGE, OP_GET_SYM_KEY, \
                            OP_GET_SYM_KEY}


/*TODO: in addition to client we will receive socket */
class Operation
{
    protected:
        uint16_t _request_code;
        Message  _msg;
        Request  _request;
        Client * _client;

    public:
        //Operation(Client * client, SOCKET sock);
        virtual std::vector<uint8_t> do_operation() = 0;
        virtual uint8_t get_operation_num() = 0;
};



class RegisterOperation : public Operation
{
    private:
        /* Request variables */
        SOCKET _interface;

        /* payload variables */
        std::string _name;
        std::string _public_key;

        bool _is_request_valid(Request * request);
        std::vector<uint8_t> _create_payload();
        RegisterOperation(Client * client, SOCKET sock);
    
    public:
        static Operation * create(Client * client, SOCKET sock)
        {
            return new RegisterOperation(client, sock);
        }

        //RegisterOperation(RegisterOperation & ro);

        std::vector<uint8_t> do_operation();

        uint8_t get_operation_num()
        {
            return OP_REGISTER;
        }

        static const size_t RESPONSE_SIZE = MIN_RESPONSE_SIZE + FIELD_CLIENT_ID_SIZE;
};



class ListUserOperation : public Operation
{
    private:
        SOCKET _interface;
        ListUserOperation(Client * client, SOCKET sock);
        void _add_contacts(std::vector<uint8_t> payload);

    public:
        static Operation * create(Client * client, SOCKET sock)
        {
            return new ListUserOperation(client, sock);
        }

        std::vector<uint8_t> do_operation();
        
        uint8_t get_operation_num()
        {
            return OP_CLIENT_LIST;
        }
};


class GetPublicKeyOperation : public Operation
{
    private:
        SOCKET _interface;
        GetPublicKeyOperation(Client * client, SOCKET sock);
        std::vector<uint8_t> _wanted_client_id;
        void _update_contact(std::vector<uint8_t> raw_data);

    public:
        static Operation * create(Client * client, SOCKET sock)
        {
            return new GetPublicKeyOperation(client, sock);
        }

        std::vector<uint8_t> do_operation();

        uint8_t get_operation_num()
        {
            return OP_PUBLIC_KEY_REQUEST;
        }

    static const size_t RESPONSE_SIZE = MIN_RESPONSE_SIZE + FIELD_CLIENT_ID_SIZE + PUBLIC_KEY_SIZE;
};


class GetSymmKeyOperation : public Operation
{
    private:
        SOCKET _interface;
        GetSymmKeyOperation(Client * client, SOCKET sock);
        std::vector<uint8_t> _wanted_client_id;
        std::vector<uint8_t> _contact_id;

    public:
        static Operation * create(Client * client, SOCKET sock)
        {
            return new GetSymmKeyOperation(client, sock);
        }

        std::vector<uint8_t> do_operation();

        uint8_t get_operation_num()
        {
            return OP_GET_SYM_KEY;
        }
    
    static const size_t RESPONSE_SIZE = MIN_RESPONSE_SIZE + FIELD_CLIENT_ID_SIZE + 4;
};


class SendSymKeyOperation : public Operation
{
    private:
        SOCKET _interface;
        SendSymKeyOperation(Client * client, SOCKET sock);
        std::vector<uint8_t> _wanted_client_id;
        std::vector<uint8_t> _contact_id;
    public:
        static Operation * create(Client * client, SOCKET sock)
        {
            return new SendSymKeyOperation(client, sock);
        }

        std::vector<uint8_t> do_operation();

        uint8_t get_operation_num()
        {
            return OP_SEND_SYM_KEY;
        }
    
    static const size_t RESPONSE_SIZE = MIN_RESPONSE_SIZE + FIELD_CLIENT_ID_SIZE + 4;

};


class SendTxtOperation : public Operation
{
    private:
        SOCKET _interface;
        SendTxtOperation(Client * client, SOCKET sock);
        std::vector<uint8_t> _wanted_client_id;
        std::vector<uint8_t> _contact_id;
    public:
        static Operation * create(Client * client, SOCKET sock)
        {
            return new SendTxtOperation(client, sock);
        }

        std::vector<uint8_t> do_operation();

        uint8_t get_operation_num()
        {
            return OP_SEND_MESSAGE;
        }
    
    static const size_t RESPONSE_SIZE = MIN_RESPONSE_SIZE + FIELD_CLIENT_ID_SIZE + 4;

};

#endif // _OPERATION_H