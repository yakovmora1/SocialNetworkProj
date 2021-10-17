class RequestStructure():
    FIELD_SIZE_CLIENT_ID = (16, 'B')
    FIELD_SIZE_CODE = (1, 'H')
    FIELD_SIZE_VERSION = (1, 'B')
    FIELD_SIZE_PAYLOAD_SIZE = (1, 'I')



for field, value in vars(RequestStructure).items():
    if field.isupper():
        print(field)
        print(value)

