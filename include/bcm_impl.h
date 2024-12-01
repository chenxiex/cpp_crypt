#pragma once
#ifndef bcm_IMPL_H
#define bcm_IMPL_H
#include "bcm.h"
#include "crypt.h"
namespace bcm
{
template <typename T> void split_input(std::vector<T> &text, const std::string &input)
{
    int length = T().size();
    if (input.size() % length != 0)
    {
        throw std::invalid_argument("Invalid input length");
    }
    text.clear();
    for (size_t i = 0; i < input.size(); i += length)
    {
        text.push_back(T(input.substr(i, length)));
    }
}
template <typename T> void merge_output(std::string &output, const std::vector<T> &text)
{
    output.clear();
    for (auto i = text.begin(); i != text.end(); i++)
    {
        output += i->to_string();
    }
}
} // namespace bcm

namespace crypt
{
template <typename BT, typename KT>
void ecb(std::string &output_string, const std::string &input_string, const KT &key,
         std::function<void(BT &output, const BT &input, const KT &key)> crypt_func)
{
    output_string.clear();
    std::vector<BT> input, output;
    bcm::split_input(input, input_string);
    output.resize(input.size());
    for (auto i = input.begin(), j = output.begin(); i != input.end() && j != output.end(); i++, j++)
    {
        crypt_func(*j, *i, key);
    }
    bcm::merge_output(output_string, output);
}

template <typename BT, typename KT>
void cbc(std::string &output_string, const std::string &input_string, const KT &key, const BT &z, const bool &decrypt,
         std::function<void(BT &output, const BT &input, const KT &key)> crypt_func)
{
    output_string.clear();
    std::vector<BT> input, output;
    bcm::split_input(input, input_string);
    output.resize(input.size());
    for (auto i = input.begin(), j = output.begin(); i != input.end() && j != output.end(); i++, j++)
    {
        if (decrypt)
        {
            crypt_func(*j, *i, key);
            if (i == input.begin())
            {
                *j ^= z;
            }
            else
            {
                *j ^= *(i - 1);
            }
        }
        else
        {
            if (i == input.begin())
            {
                BT temp = *i;
                temp ^= z;
                crypt_func(*j, temp, key);
            }
            else
            {
                BT temp = BT((*i) ^ (*(j - 1)));
                crypt_func(*j, temp, key);
            }
        }
    }
    bcm::merge_output(output_string, output);
}

template <typename BT, typename KT>
void ofb(std::string &output_string, const std::string &input_string, const KT &key, const BT &seed, const size_t &s,
         std::function<void(BT &output, const BT &input, const KT &key)> crypt_func)
{
    output_string.clear();
    if (s > BT().size() || s < 1)
    {
        throw std::invalid_argument("Invalid size");
    }
    BT r, e;
    r = seed;
    std::vector<std::string> splited_input_string;
    bcm::split_input_stream(splited_input_string, input_string, s);
    for (auto i = splited_input_string.begin(); i != splited_input_string.end(); i++)
    {
        crypt_func(e, r, key);
        BT stream_out = e;
        stream_out ^= BT(*i);
        stream_out &= BT(std::string(i->size(), '1'));
        *i = stream_out.to_string().substr(stream_out.size() - i->size(), i->size());
        r <<= s;
        r |= e & BT(std::string(s, '1'));
    }
    bcm::merge_output_stream(output_string, splited_input_string);
}

template <typename BT, typename KT>
void cfb(std::string &output_string, const std::string &input_string, const KT &key, const BT &seed, const size_t &s,
         const bool &decrypt, std::function<void(BT &output, const BT &input, const KT &key)> crypt_func)
{
    output_string.clear();
    if (s > BT().size() || s < 1)
    {
        throw std::invalid_argument("Invalid size");
    }
    BT r, e;
    r = seed;
    std::vector<std::string> splited_input_string;
    bcm::split_input_stream(splited_input_string, input_string, s);
    for (auto i = splited_input_string.begin(); i != splited_input_string.end(); i++)
    {
        crypt_func(e, r, key);
        BT stream_out = e;
        stream_out ^= BT(*i);
        stream_out &= BT(std::string(i->size(), '1'));
        if (i + 1 == splited_input_string.end())
        {
            *i = stream_out.to_string().substr(stream_out.size() - i->size(), i->size());
            break;
        }
        r <<= s;
        if (decrypt)
        {
            r |= BT(*i);
        }
        else
        {
            r |= stream_out;
        }
        *i = stream_out.to_string().substr(stream_out.size() - i->size(), i->size());
    }
    bcm::merge_output_stream(output_string, splited_input_string);
}
} // namespace crypt
#endif