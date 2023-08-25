#include "ThreadPool.h"

CThreadPool::CThreadPool(size_t num_threads)
    : num_threads_(num_threads), stop_all(false)
{
    // 스레드를 생성 및 시작한다 (시작시에는 태스크 목록이 비어있기 때문에, 모든 쓰레드는 대기상태로 전환된다)

    worker_threads_.reserve(num_threads_);

    for (size_t i = 0; i < num_threads_; ++i)
        worker_threads_.emplace_back([this]() { this->WorkerThread(); });
    /*
        thread 클래스는 생성시 태스크를 부여해야 한다.

        WorkerThread() 함수는 thread 생성시 넣어줄 함수이고, 실질적인 작업 함수는 큐에 들어있다.

        WorkerThread() 함수는 큐에 있는 작업을 꺼내 수행하는 역할을 한다.

    */
}

CThreadPool::~CThreadPool()
{
    stop_all = true;        // 스탑을 설정한다.

    cv_job_q_.notify_all(); // 모든 쓰레드를 종료시키기 위해, 모든 쓰레드를 깨운다.

    for (auto& t : worker_threads_)
        t.join();
}

void CThreadPool::WorkerThread()
{
    // 처음에 모든 쓰레드는 작업 목록이 비어 있기 때문에 대기상태로 전환된다.
    // jobs에 작업이 추가될 때까지 대기하고 있다가 작업이 추가되면, 노티파이를 통해 깬 뒤, 작업을 처리한다.

    while (true)
    {
        unique_lock<mutex> lock(m_job_q_);

        // 작업이 없거나 스탑올이 설정될 경우 대기상태로 들어간다.
        cv_job_q_.wait(lock, [this]() { return !this->jobs_.empty() || stop_all; });
        /*
            wait()
            조건이 true가 될 때까지 현재 스레드를 대기상태로 둔다.
            조건이 false라면 뮤텍스를 언락한 뒤, 대기상태로 전환된다.
        */


        // 스레드 '종료' 조건 (unique_lock은 스택 변수로서 소멸자가 호출되면 스레드가 종료된다)
        if (stop_all && this->jobs_.empty())
            return;

        // 실질적인 작업 수행
        function<void()> job = move(jobs_.front()); // 복사생성자를 호출할 수 없기 때문에 move 사용
        jobs_.pop();
        lock.unlock();

        job();
    }
}

