#include <vector>
#include <map>
#include "operation.hpp"
#include "client.hpp"

#ifndef _OPERATION_FACTORY_H
#define _OPERATION_FACTORY_H

typedef Operation * (* create_operation_fn)(Client *, SOCKET);


class OperationFactory
{
    OperationFactory();
    std::map<uint16_t, create_operation_fn> _factory_map;

    public:
    void register_op(const uint16_t op_num,
                     create_operation_fn create_func);

    static OperationFactory * get_factory()
    {
        static OperationFactory instance;
        return &instance;
    }

    Operation * create_operation(const uint16_t op_num,
                                Client * client,
                                SOCKET sock);
};

#endif //_OEPRATION_FACTORY_H