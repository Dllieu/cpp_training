#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE( Salary )

#define SALARY_TAX 0.22 // 0.17 + 0.05
#define SALARY_GROSS 10.0

BOOST_AUTO_TEST_CASE( HongKong )
{
    double rent = 13000;
    double EURtoHKD = 10.7010039;

    double rentInEUR = rent / EURtoHKD;

    double monthlyGrossSalary = SALARY_GROSS / 12;
    double monthlyNetSalary = SALARY_GROSS * ( 1 - SALARY_TAX ) / 12;

    std::cout << "Gross Salary : " << monthlyGrossSalary << " ( " << ( monthlyGrossSalary - rentInEUR )  << " )" << std::endl;
    std::cout << "Net Salary   : " << monthlyNetSalary << " ( " << ( monthlyNetSalary - rentInEUR )  << " )" << std::endl;
}

BOOST_AUTO_TEST_SUITE_END() // Salary
