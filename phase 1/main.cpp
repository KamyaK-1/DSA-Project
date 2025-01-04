#include "match_submissions.hpp"
#include "../tokenizer.hpp"
#include <iomanip>

// You should NOT modify ANYTHING in this file.
extern std::array<int, 5> match_submissions(std::vector<int> &submission1, 
        std::vector<int> &submission2);

void execute_and_verify_testcase(std::string test_dir) {
    tokenizer_t file_one(test_dir + "/one.cpp");
    tokenizer_t file_two(test_dir + "/two.cpp");
    std::vector<int> submission1 = file_one.get_tokens();
    std::vector<int> submission2 = file_two.get_tokens();
    std::array<int, 5> output = match_submissions(submission1, submission2);

    // std::ifstream in(test_dir + "/expected.txt");
    // std::array<int, 5> expected;
    // in >> expected[0] >> expected[1] >> expected[2] >> 
    //         expected[3] >> expected[4];
    // in.close();

    for(int i = 0; i < 5; i++) {
        std::cout<<"Result "<<i<<":\t";
		std::cout<<"Your output: "<<std::setw(10)<<std::left<<output[i] << std::endl;
		// std::cout<<"\tSample output: "<<expected[i]<<std::endl;
    }
	std::cout<<std::endl;
}

int main(void) {

    std::cout<<"Testcase 1: "<<std::endl;
    execute_and_verify_testcase("testcases/one");

    return 0;
}
