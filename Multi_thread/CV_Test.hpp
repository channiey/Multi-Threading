/*
	condition variable을 이용한 생산자 소비자 패턴의 멀티 쓰레딩 -> Wait의 사용 도입

    생산자가 데이터를 모아오면, 소비자가 데이터 처리를 진행한다.

    다만 아래 코드에서는 위 과정이 병렬적으로 이루어지기 위한 조건 설정을 다룬다.

    https://modoocode.com/270
    https://jungwoong.tistory.com/92
*/

#include "Defines.h"

void producer(queue<string>* downloaded_pages, mutex* m, int index, condition_variable* cv) 
{
    for (int i = 0; i < 5; i++) 
    {
        // 웹사이트를 다운로드 하는데 걸리는 시간이라 생각하면 된다.
        // 각 쓰레드 별로 다운로드 하는데 걸리는 시간이 다르다.
        this_thread::sleep_for(chrono::milliseconds(100 * index));
        string content = "Task : " + to_string(i) + " from thread(" +
            to_string(index) + ")\n";

        // data 는 쓰레드 사이에서 공유되므로 critical section 에 넣어야 한다.
        m->lock();
        downloaded_pages->push(content);
        m->unlock();

        // consumer 에게 content가 준비되었음을 알린다. (wait()함수의 조건 충족 = 깨운다)
        cv->notify_one();
    }
}

void consumer(queue<string>* downloaded_pages, mutex* m, int* num_processed, condition_variable* cv) 
{
    while (*num_processed < 25) 
    {
        unique_lock<mutex> lk(*m);
        /*
            lock_guard와 달리 생성자 말고도 함수로 락, 언락을 제공한다. 이외 기능 비슷(스택 메모리 소멸자 호출시 자동 언락)
        */

        cv->wait(
            lk, [&] { return !downloaded_pages->empty() || *num_processed == 25; });
        /*
            wait() : wait for, wait Until도 존재
            
            msdn : 조건이 true가 될 때까지 현재의 스레드를 대기상태로 둔다.

            만일 해당 조건이 거짓이라면 lk(뮤텍스)를 언락한 뒤에 누군가 깨우기 전까지 sleep상태로 둔다.
            참이라면 cv.wait은 그대로 리턴해서 consumer의 content를 처리하는 부분이 그대로 실행된다.

            해당 조건은 인자로 전달한다. (람다 사용)

            조건 해석 : downloaded_pages에 원소가 있거나 전체 처리 페이지수가 25개일 경우 wait을 중지한다.
        */

        if (*num_processed == 25) 
        {
            lk.unlock();
            return;
            //  wait 에서 탈출한 이유가 모든 페이지 처리를 완료해서 인지, 아니면 정말 downloaded_pages 에 페이지가 추가됬는지 알 수 없기 때문입니다. 
            // 만일 모든 페이지 처리가 끝나서 탈출한 것였다면, 그냥 쓰레드를 종료해야 합니다.
        }

        // 맨 앞의 페이지를 읽고 대기 목록에서 제거한다.
        string content = downloaded_pages->front();
        downloaded_pages->pop();

        (*num_processed)++;
        lk.unlock();

        // content 를 처리한다.
        cout << content;
        this_thread::sleep_for(chrono::milliseconds(80));
    }
}

void CV_Test() 
{
    // 현재 다운로드한 페이지들 리스트로, 아직 처리되지 않은 것들이다.
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
        producer 들이 모두 일을 끝낸 시점을 생각해본다면 (지금 라인), 자고 있는 일부 consumer 쓰레드들이 있을 것입니다. 
        
        만약에 cv.notify_all() 을 하지 않는다면, 자고 있는 consumer 쓰레드들의 경우 join 되지 않는 문제가 발생합니다.

        따라서 마지막으로 cv.notify_all() 을 통해서 모든 쓰레드를 깨워서 조건을 검사하도록 합니다. 
        
        해당 시점에선 이미 num_processed 가 25 가 되어 있을 것이므로, 모든 쓰레드들이 잠에서 깨어나 종료하게 됩니다.
    */
    cv.notify_all();

    for (int i = 0; i < 3; i++) 
    {
        consumers[i].join();
    }
}