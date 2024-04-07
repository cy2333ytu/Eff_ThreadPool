#ifndef EXCEPTION_H
#define EXCEPTION_H

#include "StrDefine.h"

#include <string>
#include <exception>

namespace ccy
{

class EXCEPTION : public std::exception {
public:
    explicit EXCEPTION(const std::string& info,
                        const std::string& locate = "") {
        exception_info_ = locate + " | " + info;
    }

    /**
     * 获取异常信息
     * @return
     */
    const char* what() const noexcept override {
        return exception_info_.c_str();
    }

private:
    std::string exception_info_;            // 异常状态信息
};

}

#endif 
