#include "gtest/gtest.h"
#include "bayan.h"
#include <iostream>

class TestController : public ::testing::Test 
{
	std::streambuf* old_stdout{nullptr};
protected:
	void SetUp() 
	{
		old_stdout = std::cout.rdbuf(new_stdout.rdbuf());
	}

	void TearDown() 
	{
		old_stdout = std::cout.rdbuf(old_stdout);
	}
	int argc = 4;
	char* argv[4]= {
		"bayan",
		"-l",
		"1",
	#if defined(WIN32)
		"..\\test"
	#else
		"../test"
	#endif
	};
	bayan::Bayan instance;
	std::stringstream new_stdout;
};

TEST_F(TestController, test_bayan)
{	
    instance.getDublicates(argc, argv);  
#if defined(WIN32)
	ASSERT_EQ(new_stdout.str(), "..\\test\\2\\1\\2.txt\n..\\test\\2\\1.log\n..\\test\\1\\2\\2.doc\n\n..\\test\\1.html\n..\\test\\1\\2.php\n\n");
#else
	EXPECT_TRUE(
		new_stdout.str() == "../test/2/1.log\n../test/2/1/2.txt\n../test/1/2/2.doc\n\n../test/1.html\n../test/1/2.php\n\n" ||
		new_stdout.str() == "../test/2/1/2.txt\n../test/2/1.log\n../test/1/2/2.doc\n\n../test/1.html\n../test/1/2.php\n\n" 
	);
#endif
}