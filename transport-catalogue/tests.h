#pragma once
#include <string>
#include <cassert>
#include <iostream>
#include <sstream>
#include "transport_catalogue.h"

template <typename T, typename U>
void AssertEqualImpl(const T& t, const U& u, const std::string& t_str, const std::string& u_str, const std::string& file,
    const std::string& func, unsigned line, const std::string& hint);

void AssertImpl(bool value, const std::string& expr_str, const std::string& file, const std::string& func, unsigned line,
    const std::string& hint);

template <typename T>
void RunTestImpl(const T&, const std::string& t_str);

template <typename T, typename U>
void AssertEqualImpl(const T& t, const U& u, const std::string& t_str, const std::string& u_str, const std::string& file,
    const std::string& func, unsigned line, const std::string& hint) {
    if (t != u) {
        std::cout << std::boolalpha;
        std::cout << file << "(" << line << "): " << func << ": ";
        std::cout << "ASSERT_EQUAL(" << t_str << ", " << u_str << ") failed: ";
        std::cout << t << " != " << u << ".";
        if (!hint.empty()) {
            std::cout << " Hint: " << hint;
        }
        std::cout << std::endl;
        abort();
    }
}

void TestAddingInput();

void TestAddingStat();

void TestGettingResult();

void TestTransportCatalogue();

template <typename T>
void RunTestImpl(const T& func, const std::string& t_str) {
    func();
    std::cerr << t_str << " OK" << std::endl;
}