#include "tests.h"

#define ASSERT_EQUAL(a, b) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, "")

#define ASSERT_EQUAL_HINT(a, b, hint) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, (hint))

void AssertImpl(bool value, const std::string& expr_str, const std::string& file, const std::string& func, unsigned line,
    const std::string& hint) {
    if (!value) {
        std::cout << file << "(" << line << "): " << func << ": ";
        std::cout << "ASSERT(" << expr_str << ") failed.";
        if (!hint.empty()) {
            std::cout << " Hint: " << hint;
        }
        std::cout << std::endl;
        abort();
    }
}

#define ASSERT(expr) AssertImpl((expr), #expr, __FILE__, __FUNCTION__, __LINE__, "")

#define ASSERT_HINT(expr, hint) AssertImpl((expr), #expr, __FILE__, __FUNCTION__, __LINE__, (hint))

#define RUN_TEST(func) RunTestImpl((func), #func)

void TestAddingInput() {
    std::istringstream input{
        "11\n"
        "Stop Tolstopaltsevo: 55.611087, 37.208290                    \n"
        "Stop                                          Marushkino: 55.595884, 37.209755              \n"
        "Bus 256: Biryulyovo Zapadnoye > Biryusinka > Universam                           > Biryulyovo Tovarnaya > Biryulyovo Passazhirskaya > Biryulyovo Zapadnoye\n"
        "Bus Raz Dva: Tolstopaltsevo -                         Marushkino - Rasskazovka\n"
        "Stop Rasskazovka: 55.632761,                     37.333324\n"
        "Stop Biryulyovo Zapadnoye: 55.574371, 37.651700           \n"
        "Stop Biryusinka: 55.581065, 37.648390\n"
        "Stop Universam:                          55.587655, 37.645687\n"
        "Stop                      Biryulyovo Tovarnaya: 55.592028, 37.653656\n"
        "Bus Raz                          Dva: Tolstopaltsevo -                         Marushkino - Rasskazovka\n"
        "Stop Biryulyovo Passazhirskaya: 55.580999, 37.659164\n"
    };
    {
        input::Requests inp;
        inp.Load(input);
        auto result = inp.GetRequests();
        ASSERT_EQUAL(result.size(), 11);
        ASSERT(result[0].type == input::RequestType::STOP);
        ASSERT(result[9].type == input::RequestType::BUS);
        ASSERT_EQUAL(result[6].text, "Biryusinka: 55.581065, 37.648390");
    }
}

void TestAddingStat() {
    std::istringstream input{
        "9\n"
        "Bus 256\n"
        "Bus Raz Dva\n"
        "Bus 751\n"
        "Bus             a\n"
        "Bus              \n"
        "          Bus Raz Dva                   \n"
        "Bus Raz                          Dva\n"
        "                                          \n"
        "\n"
    };
    {
        stat_read::Requests stt;
        stt.Load(input);
        auto result = stt.GetRequests();
        ASSERT_EQUAL(result.size(), 6);
        ASSERT(result[3].type == stat_read::RequestType::BUS);
        ASSERT(result[0].text == "256");
        ASSERT_EQUAL(result[4].text, "Raz Dva");
    }
}

void TestGettingResult() {
    std::istringstream input{
        "11\n"
        "Stop Stop name 1: 55.611087, 37.208290                    \n"
        "Stop                                          Marushkino: 55.595884, 37.209755              \n"
        "Bus 256: Biryulyovo Zapadnoye > Biryusinka > Universam                           > Biryulyovo Tovarnaya > Biryulyovo Passazhirskaya > Biryulyovo Zapadnoye\n"
        "Bus Raz Dva: Stop name 1 -                         Marushkino - Rasskazovka\n"
        "Stop Rasskazovka: 55.632761,                     37.333324\n"
        "Stop Biryulyovo Zapadnoye: 55.574371, 37.651700           \n"
        "Stop Biryusinka: 55.581065, 37.648390\n"
        "Stop Universam:                          55.587655, 37.645687\n"
        "Stop                      Biryulyovo Tovarnaya: 55.592028, 37.653656\n"
        "Bus Raz                          Dva: Stop name 1                                                     - Marushkino - Rasskazovka\n"
        "Stop Biryulyovo Passazhirskaya: 55.580999, 37.659164\n"
        "9\n"
        "Bus 256\n"
        "Bus Raz Dva\n"
        "Bus 751\n"
        "Bus             a\n"
        "Bus              \n"
        "          Bus Raz Dva                   \n"
        "Bus Raz                          Dva\n"
        "                                          \n"
        "\n"
    };
    {
        input::Requests inp;
        inp.Load(input);
        stat_read::Requests stt;
        stt.Load(input);
        TransportCatalogue tc;
        tc.ProcessInput(inp);
        tc.ProcessStats(stt);
        auto result = stt.GetRepliesBus();
        ASSERT_EQUAL(result.size(), 6);
        ASSERT_EQUAL(result[2].name, "751");
        ASSERT_EQUAL(result[1].stops, result[4].stops);
        ASSERT_EQUAL(result[5].unique_stops, 3);
        ASSERT_EQUAL(result[2].stops, 0);
    }
}

void TestTransportCatalogue() {
    RUN_TEST(TestAddingInput);
    RUN_TEST(TestAddingStat);
    //RUN_TEST(TestGettingResult);
}