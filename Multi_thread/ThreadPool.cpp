#include "ThreadPool.h"

CThreadPool::CThreadPool(size_t num_threads)
    : num_threads_(num_threads), stop_all(false)
{
    // �����带 ���� �� �����Ѵ� (���۽ÿ��� �½�ũ ����� ����ֱ� ������, ��� ������� �����·� ��ȯ�ȴ�)

    worker_threads_.reserve(num_threads_);

    for (size_t i = 0; i < num_threads_; ++i)
        worker_threads_.emplace_back([this]() { this->WorkerThread(); });
    /*
        thread Ŭ������ ������ �½�ũ�� �ο��ؾ� �Ѵ�.

        WorkerThread() �Լ��� thread ������ �־��� �Լ��̰�, �������� �۾� �Լ��� ť�� ����ִ�.

        WorkerThread() �Լ��� ť�� �ִ� �۾��� ���� �����ϴ� ������ �Ѵ�.

    */
}

CThreadPool::~CThreadPool()
{
    stop_all = true;        // ��ž�� �����Ѵ�.

    cv_job_q_.notify_all(); // ��� �����带 �����Ű�� ����, ��� �����带 �����.

    for (auto& t : worker_threads_)
        t.join();
}

void CThreadPool::WorkerThread()
{
    // ó���� ��� ������� �۾� ����� ��� �ֱ� ������ �����·� ��ȯ�ȴ�.
    // jobs�� �۾��� �߰��� ������ ����ϰ� �ִٰ� �۾��� �߰��Ǹ�, ��Ƽ���̸� ���� �� ��, �۾��� ó���Ѵ�.

    while (true)
    {
        unique_lock<mutex> lock(m_job_q_);

        // �۾��� ���ų� ��ž���� ������ ��� �����·� ����.
        cv_job_q_.wait(lock, [this]() { return !this->jobs_.empty() || stop_all; });
        /*
            wait()
            ������ true�� �� ������ ���� �����带 �����·� �д�.
            ������ false��� ���ؽ��� ����� ��, �����·� ��ȯ�ȴ�.
        */


        // ������ '����' ���� (unique_lock�� ���� �����μ� �Ҹ��ڰ� ȣ��Ǹ� �����尡 ����ȴ�)
        if (stop_all && this->jobs_.empty())
            return;

        // �������� �۾� ����
        function<void()> job = move(jobs_.front()); // ��������ڸ� ȣ���� �� ���� ������ move ���
        jobs_.pop();
        lock.unlock();

        job();
    }
}

