/*

	������ Ǯ�� ������ �� �ִ� �׽�Ʈ ����

    ������ Ǯ�� ���� �����ϴ� �½�ũ �Լ��� ���ϰ��� �����ϴ� ��쿡 ���� ����

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
    CThreadPool pool(3); // 3���� ������ ����

    vector<future<int>> futures; // �����忡 �ο��ϴ� �½�ũ �Լ��� ��ȯ���� �����ϴ� ��츦 ����� ��ȯ���� ���� �� �ֵ��� �����Ѵ�.

    cout << "Thread Start\n\n";

    for (int i = 0; i < 10; i++)  // 3���� �����忡 ���� �� 10���� Ŀ�ǵ�(������ ���ϴ� �Լ�) �ο�
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