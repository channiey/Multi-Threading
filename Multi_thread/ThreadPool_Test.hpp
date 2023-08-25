/*

	쓰레드 풀을 실행할 수 있는 테스트 파일

    쓰레드 풀을 통해 실행하는 태스크 함수에 리턴값이 존재하는 경우에 대한 예제

*/

#include "ThreadPool.h"

int work(int t, int id) 
{

    printf("%d start \n", id);
    this_thread::sleep_for(chrono::seconds(t));
    printf("%d end\n", id);

    return t + id;
}

void ThreadPool_Test() 
{
    CThreadPool pool(3); // 3개의 쓰레드 생성

    vector<future<int>> futures; // 쓰레드에 부여하는 태스크 함수가 반환값이 존재하는 경우를 대비해 반환값을 받을 수 있도록 구현한다.

    cout << "Thread Start\n\n";

    for (int i = 0; i < 10; i++)  // 3개에 쓰레드에 대해 총 10개의 커맨드(실행을 원하는 함수) 부여
    {
        futures.emplace_back(pool.EnqueueJob(work, i % 3 + 1, i));
    }

    for (auto& f : futures) 
    {
        printf("result : %d \n", f.get());
    }

    cout << "\nThread Exit\n";

}


//ThreadPool_Test