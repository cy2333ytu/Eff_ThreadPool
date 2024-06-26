#ifndef STATUS_H
#define STATUS_H

#include "StrDefine.h"

#include <string>

namespace ccy
{
/**
 * 说明：
 * 返回值为0，表示正常逻辑
 * 返回值为负整数，表示error逻辑，程序终止执行
 */
static const int STATUS_OK = 0;                                 /** 正常流程返回值 */
static const int STATUS_ERR = -1;                               /** 异常流程返回值 */
static const int STATUS_CRASH = -996;                           /** 异常流程返回值 */
static const char* STATUS_ERROR_INFO_CONNECTOR = " && ";        /** 多异常信息连接符号 */

class STATUS {
public:
    explicit STATUS() = default;

    explicit STATUS(const std::string &errorInfo,
                     const std::string &locateInfo = EMPTY) {
        this->error_code_ = STATUS_ERR;    // 默认的error code信息
        this->error_info_ = errorInfo;
        this->error_locate_ = locateInfo;
    }

    explicit STATUS(int errorCode, const std::string &errorInfo,
                     const std::string &locateInfo = EMPTY) {
        this->error_code_ = errorCode;
        this->error_info_ = errorInfo;
        this->error_locate_ = locateInfo;
    }

    STATUS(const STATUS &status) {
        if (status.isOK()) {
            return;
        }

        this->error_code_ = status.error_code_;
        this->error_info_ = status.error_info_;
        this->error_locate_ = status.error_locate_;
    }

    STATUS(const STATUS &&status) noexcept {
        if (status.isOK()) {
            return;
        }

        this->error_code_ = status.error_code_;
        this->error_info_ = status.error_info_;
        this->error_locate_ = status.error_locate_;
    }

    STATUS& operator=(const STATUS& status) {
        if (!status.isOK()) {
            // 如果status是正常的话，则所有数据保持不变
            this->error_code_ = status.error_code_;
            this->error_info_ = status.error_info_;
            this->error_locate_ = status.error_locate_;
        }
        return (*this);
    }

    STATUS& operator+=(const STATUS& cur) {
        /**
         * 如果当前状态已经异常，则不做改动
         * 如果当前状态正常，并且传入的状态是异常的话，则返回异常
         */
        if (!this->isErr() && cur.isErr()) {
            this->error_code_ = cur.error_code_;
            this->error_info_ = cur.error_info_;
            this->error_locate_ = cur.error_locate_;
        }

        return (*this);
    }

    /**
     * 获取异常值信息
     * @return
     */
    int getCode() const {
        return this->error_code_;
    }

    /**
     * 获取异常信息
     * @return
     */
    const std::string& getInfo() const {
        return this->error_info_;
    }

    /**
     * 获取报错位置
     * @return
     */
    const std::string& getLocate() const {
        return this->error_locate_;
    }

    /**
     * 判断当前状态是否可行
     * @return
     */
    bool isOK() const {
        return STATUS_OK == error_code_;
    }

    /**
     * 判断当前状态是否可行
     * @return
     */
    bool isErr() const {
        return error_code_ < STATUS_OK;    // 约定异常信息，均为负值
    }

    /**
     * 判断当前状态是否是崩溃了
     * @return
     */
    bool isCrash() const {
        return STATUS_CRASH == error_code_;
    }

private:
    int error_code_ = STATUS_OK;                     // 错误码信息
    std::string error_info_;                         // 错误信息描述
    std::string error_locate_;                       // 错误发生的具体位置，如：file|function|line
};

using Status = STATUS;

}

#endif 
