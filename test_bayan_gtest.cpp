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
		"test"
	#endif
	};
	bayan::Bayan instance;
	std::stringstream new_stdout;
};

TEST_F(TestController, test_bayan)
{	
    instance.getDublicates(argc, argv);  
#if defined(WIN32)
		EXPECT_TRUE(
		new_stdout.str() == "..\\test\\1.html\n..\\test\\1\\2.php\n\n" ||
		new_stdout.str() == "..\\test\\1\\2.php\n..\\test\\1.html\n\n" 
	);
#else
	EXPECT_TRUE(
		new_stdout.str() == "test/1.html\ntest/1/2.php\n\n" ||
		new_stdout.str() == "test/1/2.php\ntest/1.html\n\n" 
	);
#endif
}

int main(int argc, char *argv[])
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}