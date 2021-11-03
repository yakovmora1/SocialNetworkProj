#include "operation_factory.hpp"
#include <iostream>


OperationFactory::OperationFactory()
{
    /* register all the supported operations */
    register_op(OP_REGISTER, RegisterOperation::create);
    //std::cout << "Registered RegisterOperation " << std::endl;
    register_op(OP_CLIENT_LIST, ListUserOperation::create);
    //std::cout << "Registered ListUserOperation " << std::endl;
    register_op(OP_PUBLIC_KEY_REQUEST, GetPublicKeyOperation::create);
    //std::cout << "Registered GetPublicKeyOperation " << std::endl;
    register_op(OP_GET_SYM_KEY, GetSymmKeyOperation::create);
    //std::cout << "Registered GetSymmKeyOperation " << std::endl;
    register_op(OP_SEND_SYM_KEY, SendSymKeyOperation::create);
    //std::cout << "Registered SendSymKeyOperation " << std::endl;
    register_op(OP_SEND_MESSAGE, SendTxtOperation::create);
    //std::cout << "Registered SendTxtOperation " << std::endl;
    register_op(OP_FETCH_MESSAGES, FetchMsgOperation::create);
    //std::cout << "Registered FetchMsgOperation " << std::endl;
}

void OperationFactory::register_op(const uint16_t op_num,
                            create_operation_fn create_func)
{
    _factory_map[op_num] = create_func;
}


Operation * OperationFactory::create_operation(const uint16_t op_num,
                                                Client * client,
                                                SOCKET socket)
{
    std::map<uint16_t, create_operation_fn>::iterator it;
    it = _factory_map.find(op_num);

    if(it != _factory_map.end())
    {
        std::cout << "Retreiving Operation : " << op_num << std::endl;
        return it->second(client, socket);
    }
    return nullptr;
}