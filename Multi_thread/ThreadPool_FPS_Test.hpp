/*

	쓰레드 풀을 실행할 수 있는 테스트 파일

	쓰레드 풀을 통해 실행하는 태스크를 쓰레드 개수별 처리시간을 비교한다.

*/

#include "ThreadPool.h"

int Fibonacci(int num)
{
	if (num == 1)
		return 1;

	if (num == 2)
		return 1;

	return Fibonacci(num - 1) + Fibonacci(num - 2);
}

void Execute_FPS_Test()
{
	clock_t  tStartTime, tFinishTime;
	double dDurationTime;

	const int iMaxThread		= 8;
	const int iNumOfFibonacci	= 20;
	const int iNumOfCommand		= 100000;
	double dPrevDurationTime	= 0.f;

	cout << "Num of Command (Fibonacci) : " << iNumOfCommand << endl << endl;
	cout << "===================================\n\n";
	for (int i = 1; i <= iMaxThread; ++i)
	{
		cout << "Num of Thread : \t\t" << i << "\tthread\n";

		{
			CThreadPool			pool(i);
			vector<future<int>> futures;

			//cout << "Thread Start\n\n";
			tStartTime = clock();

			for (int i = 0; i < iNumOfCommand; i++)
				futures.emplace_back(pool.EnqueueJob(Fibonacci, iNumOfFibonacci));
		}

		tFinishTime = clock();
		//cout << "\nThread Exit\n";

		dDurationTime = (double)(tFinishTime - tStartTime);
		cout << "Laps : \t\t\t\t" << dDurationTime << "\tms\n\n";
		/*if (0.f != dPrevDurationTime)
		{
			float dRateOfIncrease = ((dDurationTime - dPrevDurationTime) / dPrevDurationTime) * -100.f;
			cout << "Performance Growth Rate : \t" << dRateOfIncrease << "\t%\n\n";
		}
		else
		{
			cout << "\n";
		}
		dPrevDurationTime = dDurationTime;*/

	}


	int k = 0;
	cin >> k;

	return;
}