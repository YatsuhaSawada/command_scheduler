#include "pch.h"
#include <stdio.h>
#include "Command.hpp"
#include "CommandScheduler.hpp"
#include <functional>

class Result
{
public:

	enum Code
	{
		OK,
		NG,
		Cancel,
	};

public:

	//lifeCycle
	explicit Result(const Code code);

	Result(const Result& rhs) = default;

	Result& operator=(Result&&) = default;

public:

	//propaty
	Code code() const;

public:

	Result& operator=(const Result& rhs) = default;

private:

	Code _code;
};

//===========================================
//lifeCycle
//===========================================
Result::Result(const Code code)
	: _code(code)
{
}

//===========================================
//propaty
//===========================================
Result::Code Result::code() const
{
	return _code;
}
class SampleServices
{
public:

	Result countUp(long& s)
	{
		long i = s;
		s = ++i;
		return Result(Result::OK);
	}

	Result countUpLoop(long s)
	{
		_cencal = false;
		long i = s;
		for (;;)
		{
			std::cout << "count " << i++ << std::endl;

			//Sleep(100);

			if (true == _cencal)
			{
				return Result(Result::Cancel);
			}
		}

		return Result(Result::OK);
	}

	template <class CB>
	Result countUpLoopCB(long s, CB cb)
	{
		_cencal = false;
		long i = s;
		for (;;)
		{
			std::cout << "count " << i++ << std::endl;

			
			cb(i);

			if (true == _cencal)
			{
				return Result(Result::Cancel);
			}
		}

		return Result(Result::OK);
	}

	void callBack(Result result)
	{
		std::cout << "result " << result.code() << std::endl;
	}

	Result cancel()
	{
		_cencal = true;

		return Result(Result::OK);
	}

private:

	bool _cencal;

};
TEST(TestCaseName, TestName) {

	SampleServices sample;

	CommandScheduler < Result > thread;
	thread.start("testThread");

	//thread.add(createCommand<Result>(
	//	std::bind(&SampleServices::countUpLoop, &sample, 5),
	//	std::bind(&SampleServices::cancel, &sample)
	//	));

	thread.add(createCommand<Result>(
		std::bind([&](auto... args) { return sample.countUpLoopCB(args...); }, 5, [](int s) { std::cout << "result " << s << std::endl; }),
		std::bind(&SampleServices::cancel, &sample)
		));

	for (;;)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1200)); // 3 ƒ~ƒŠ•b
		thread.cancelCurrent();

		if (true == thread.empty())
		{
			break;
		}
	}

  EXPECT_EQ(1, 1);
  EXPECT_TRUE(true);
}