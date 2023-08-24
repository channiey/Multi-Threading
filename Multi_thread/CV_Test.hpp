/*
	condition variable�� �̿��� ������ �Һ��� ������ ��Ƽ ������ -> Wait�� ��� ����

    �����ڰ� �����͸� ��ƿ���, �Һ��ڰ� ������ ó���� �����Ѵ�.

    �ٸ� �Ʒ� �ڵ忡���� �� ������ ���������� �̷������ ���� ���� ������ �ٷ��.

    https://modoocode.com/270
    https://jungwoong.tistory.com/92
*/

#include "Defines.h"

void producer(queue<string>* downloaded_pages, mutex* m, int index, condition_variable* cv) 
{
    for (int i = 0; i < 5; i++) 
    {
        // ������Ʈ�� �ٿ�ε� �ϴµ� �ɸ��� �ð��̶� �����ϸ� �ȴ�.
        // �� ������ ���� �ٿ�ε� �ϴµ� �ɸ��� �ð��� �ٸ���.
        this_thread::sleep_for(chrono::milliseconds(100 * index));
        string content = "Task : " + to_string(i) + " from thread(" +
            to_string(index) + ")\n";

        // data �� ������ ���̿��� �����ǹǷ� critical section �� �־�� �Ѵ�.
        m->lock();
        downloaded_pages->push(content);
        m->unlock();

        // consumer ���� content�� �غ�Ǿ����� �˸���. (wait()�Լ��� ���� ���� = �����)
        cv->notify_one();
    }
}

void consumer(queue<string>* downloaded_pages, mutex* m, int* num_processed, condition_variable* cv) 
{
    while (*num_processed < 25) 
    {
        unique_lock<mutex> lk(*m);
        /*
            lock_guard�� �޸� ������ ���� �Լ��� ��, ����� �����Ѵ�. �̿� ��� ���(���� �޸� �Ҹ��� ȣ��� �ڵ� ���)
        */

        cv->wait(
            lk, [&] { return !downloaded_pages->empty() || *num_processed == 25; });
        /*
            wait() : wait for, wait Until�� ����
            
            msdn : ������ true�� �� ������ ������ �����带 �����·� �д�.

            ���� �ش� ������ �����̶�� lk(���ؽ�)�� ����� �ڿ� ������ ����� ������ sleep���·� �д�.
            ���̶�� cv.wait�� �״�� �����ؼ� consumer�� content�� ó���ϴ� �κ��� �״�� ����ȴ�.

            �ش� ������ ���ڷ� �����Ѵ�. (���� ���)

            ���� �ؼ� : downloaded_pages�� ���Ұ� �ְų� ��ü ó�� ���������� 25���� ��� wait�� �����Ѵ�.
        */

        if (*num_processed == 25) 
        {
            lk.unlock();
            return;
            //  wait ���� Ż���� ������ ��� ������ ó���� �Ϸ��ؼ� ����, �ƴϸ� ���� downloaded_pages �� �������� �߰������ �� �� ���� �����Դϴ�. 
            // ���� ��� ������ ó���� ������ Ż���� �Ϳ��ٸ�, �׳� �����带 �����ؾ� �մϴ�.
        }

        // �� ���� �������� �а� ��� ��Ͽ��� �����Ѵ�.
        string content = downloaded_pages->front();
        downloaded_pages->pop();

        (*num_processed)++;
        lk.unlock();

        // content �� ó���Ѵ�.
        cout << content;
        this_thread::sleep_for(chrono::milliseconds(80));
    }
}

void CV_Test() 
{
    // ���� �ٿ�ε��� �������� ����Ʈ��, ���� ó������ ���� �͵��̴�.
    queue<string> downloaded_pages;
    mutex m;
    condition_variable cv;

    vector<thread> producers;
    for (int i = 0; i < 5; i++) 
    {
        producers.push_back(
            thread(producer, &downloaded_pages, &m, i + 1, &cv));
    }

    int num_processed = 0;
    vector<thread> consumers;
    for (int i = 0; i < 3; i++) 
    {
        consumers.push_back(
            thread(consumer, &downloaded_pages, &m, &num_processed, &cv));
    }

    for (int i = 0; i < 5; i++) 
    {
        producers[i].join();
    }

    /*
        producer ���� ��� ���� ���� ������ �����غ��ٸ� (���� ����), �ڰ� �ִ� �Ϻ� consumer ��������� ���� ���Դϴ�. 
        
        ���࿡ cv.notify_all() �� ���� �ʴ´ٸ�, �ڰ� �ִ� consumer ��������� ��� join ���� �ʴ� ������ �߻��մϴ�.

        ���� ���������� cv.notify_all() �� ���ؼ� ��� �����带 ������ ������ �˻��ϵ��� �մϴ�. 
        
        �ش� �������� �̹� num_processed �� 25 �� �Ǿ� ���� ���̹Ƿ�, ��� ��������� �ῡ�� ��� �����ϰ� �˴ϴ�.
    */
    cv.notify_all();

    for (int i = 0; i < 3; i++) 
    {
        consumers[i].join();
    }
}