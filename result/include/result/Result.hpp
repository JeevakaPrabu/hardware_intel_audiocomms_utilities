/**
 * @section License
 *
 * Copyright 2013-2014 Intel Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once
#include <string>

#include <sstream>

namespace audio_comms
{
namespace utilities
{
namespace result
{

/** A result that carries a error code describing the error.
 *
 * @tparam ErrorTrait a class describing a specific error field
 *  ErrorTrait MUST have a Code type which hold its error values and MUST have
 *  codeToString() static function that converts this code type into std::string.
 *
 * Can be cast to bool to see if the function succeed.
 */
template <class ErrorTrait>
class Result
{
public:
    typedef ErrorTrait Trait;
    /** Result constructor
     *
     * this creates a Result object which holds the given error code.
     *
     * @param[in] code the code to be put in Result; Default value is
     *                       ErrorTrait::defaultError.
     */
    explicit Result(typename ErrorTrait::Code code = ErrorTrait::defaultError)
        : _errorCode(code)
    {}

    /** Result constructor from an other result
     *
     * Construct a result where it's
     *     error_code = inputResult.isFailure() ? failureCode : successCode
     * If input result is a failure, append it (formated) to the message.
     *
     * @tparam InputTrait is the trait of the input result
     * @param inputResult is the input result
     * @param failureCode is the error code to use if the input result is a failure
     * @param successCode is the error code to use if the input result is a success
     */
    template <class InputTrait>
    explicit Result(const Result<InputTrait> &inputResult,
                    typename ErrorTrait::Code failureCode,
                    typename ErrorTrait::Code successCode = ErrorTrait::success)
    {
        if (inputResult.isFailure()) {
            _errorCode = failureCode;
            *this << inputResult;
        } else {
            _errorCode = successCode;
        }
    }

    /** Get the error code.
     *
     * @return the error code, undefined behaviour if the result is a success.
     */
    typename ErrorTrait::Code getErrorCode() const { return _errorCode; }

    /** Get the error message
     * @return the error message held by the class
     */
    const std::string &getMessage() const { return _message; }

    /**
     * Comparison operator.
     * Careful, comparison is performed on error code only, messages MAY differ
     * even if this function returns true.
     *
     * @param[in] toCompare  a Coderesult to compare with us
     *
     * @return true on equality, false otherwise
     */
    bool operator==(const Result<ErrorTrait> &toCompare) const
    {
        return _errorCode == toCompare._errorCode;
    }

    /**
     * Success
     * This function alway return a successful Result
     *
     * @return a successful coderesult
     */
    static const Result<ErrorTrait> &success()
    {
        static Result<ErrorTrait> success(ErrorTrait::success);
        return success;
    }

    /**
     * is object holding a Success value
     *
     * @return true is the error code is success, false otherwise
     */
    bool isSuccess() const { return _errorCode == ErrorTrait::success; }

    /**
     * Cast to ErrorTrait::Code
     * This function allows to test a Result with an ErrorTrait::Code easily
     *
     * @return the code held by the object
     */
    operator typename ErrorTrait::Code() const {
        return _errorCode;
    }

    /**
     * is object holding a failure
     *
     * @return true if error code is NOT a success
     */
    bool isFailure() const { return !isSuccess(); }

    /**
     * Format a Result in oder to have it under a synthetic human readable view.
     *
     * @return a formated Result
     */
    std::string format() const
    {
        if (isFailure()) {
            std::ostringstream ss;

            // error code and error description
            ss << "Code " << static_cast<int>(_errorCode) << ": " <<
                ErrorTrait::codeToString(_errorCode);

            // concatenate the message (if not empty)
            if (_message.length() > 0) {
                ss << " (" << _message << ")";
            }
            return ss.str();
        }
        return "Success";
    }

    /**
     * Appends any data to message
     * Careful: this function expects that T has a known conversion to ostream
     *          or a Result
     *
     * @tparam T   type of the thing to append
     * @param[in]  aT  a T thing to append
     *
     * @return a copy of *this
     */
    template <class T>
    Result<ErrorTrait> operator<<(const T &aT)
    {
        Append<T>::run(*this, aT);
        return *this;
    }

private:
    /** Artefact class to partial specialize operator<< */
    template <class Data>
    struct Append
    {
        /** Append a data to _message. @see Result::operator<< */
        static void run(Result<ErrorTrait> &res, const Data &data)
        {
            std::ostringstream ss;
            ss << data;
            res._message += ss.str();
        }
    };
    template <class Err>
    struct Append<Result<Err> >
    {
        /** Append a Result to _message. @see Result::operator<< */
        static void run(Result<ErrorTrait> &res, const Result<Err> &data)
        {
            if (not res._message.empty()) {
                res._message += ": ";
            }
            res._message += data.format();
        }
    };
    template <class>
    template <class Data>
    friend void Append<Data>::run();

    typename ErrorTrait::Code _errorCode; /*< result code held by class */
    std::string _message;                 /*< an error message explaining the error */
};

} /* namespace result */

} /* namespace utilities */

} /* namespace audio_comms */