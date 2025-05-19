#include "DatabaseException.h"

DatabaseException::DatabaseException(const std::string& message)
    : runtime_error(message) {}