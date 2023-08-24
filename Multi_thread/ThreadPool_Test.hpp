/*

	������ Ǯ

	������Ʈ Ǯ�� �����ϴ�. ���� ���� ��������� ����ϰ� �ִٰ� �½�ũ�� ������ �ȴٸ�, ����ϴ� �������� �ϳ��� �̸� �޾Ƽ� �����Ѵ�.

    // �������� �浹ó���� ��� ���� -> �ھ �ִ� ������ Ȱ��

    https://modoocode.com/285

*/

#include "Defines.h"

namespace ThreadPool 
{
    class ThreadPool 
    {
        public:
            ThreadPool(size_t num_threads);
            ~ThreadPool();

            // job �� �߰��Ѵ�.
            // EnqueJob()�Լ��� ������ ������ �Լ��� �ް� �� �Լ��� ���ϰ��� �����ϴ� feture�� �����ϵ��� �Ѵ�. feture�� ���� ���� <>�ȿ� ����.
            template <class F, class... Args> // ���� ���� ���ø�
            future<typename result_of<F(Args...)>::type> EnqueueJob(F&& f, Args&&... args); // �Լ� F�� ���ϰ��� �˱����� result_of�� ����Ѵ�.
            // typename std::result_of<F(Args...)>::type  ==  f �� ���ϰ�

            // ���� ���Ǵ� �Ʒ��� ����.
            // template <class F, class... Args>
            // std::future</* f �� ���� Ÿ��*/> EnqueueJob(F f, Args... args);

        private:
            // Worker ������
            void WorkerThread();

        private:
            // �� Worker �������� ����.
            size_t num_threads_;

            // Worker �����带 �����ϴ� ����.
            vector<thread> worker_threads_;

            // ���ϵ��� �����ϴ� job ť. -> Ŀ�ǵ� ����Ʈ (�����ؾ��� �Լ��� �����Ѵ�, function�� �Լ��� ��ü�� �����ϱ� ���� ���)
            // ��� �۾� ������鿡�� ������ �����ϴ�. ���� �Ʒ� ��ġ�� ��ȣ
            queue<function<void()>> jobs_;

            // ���� job ť�� ���� cv �� m.
            condition_variable cv_job_q_; // (������ ������ ������Ǯ�� ����ϴ� ����� = �۾��� �߰��ϴ� ����� / �Һ��ڵ��� ������)
            mutex m_job_q_;

            // ��� �����带 �����ϱ� ����
            // ��Ŀ ��������� �⺻������ jobs�� ó���ϴ� ���� ���ѷ����� ���� �ִµ� stop_All�� �����ȴٸ� ���ѷ����� ���������� �ȴ�.
            bool stop_all;
    };

    ThreadPool::ThreadPool(size_t num_threads)
        : num_threads_(num_threads), stop_all(false) 
    {
        // �����带 ���� �� �����Ѵ�.

        worker_threads_.reserve(num_threads_);

        for (size_t i = 0; i < num_threads_; ++i) 
        {
            worker_threads_.emplace_back([this]() { this->WorkerThread(); });
        }
    }

    void ThreadPool::WorkerThread() 
    {
        // jobs�� �۾��� �߰��� ������ ����ϰ� �ִٰ� �۾��� �߰��Ǹ� �޾Ƽ� ó���Ѵ�.
        while (true) 
        {
            unique_lock<mutex> lock(m_job_q_);

            // �۾��� ���ų� ��ž���� ������ ��� �����·� ����.
            cv_job_q_.wait(lock, [this]() { return !this->jobs_.empty() || stop_all; });
            
            // ������ ���� ���� (unique_lock�� ���� �����μ� �Ҹ��ڰ� ȣ��Ǹ� �����尡 ����ȴ�)
            if (stop_all && this->jobs_.empty())  
                return;
            
            // �� ���� job �� ����.
            function<void()> job = move(jobs_.front()); // ��������ڸ� ȣ���� �� ���� ������ move ���
            jobs_.pop();
            lock.unlock();

            // �ش� job �� �����Ѵ� :)
            job();
        }
    }


    template <class F, class... Args>
    future<typename result_of<F(Args...)>::type> ThreadPool::EnqueueJob(F&& f, Args&&... args) 
    {
        if (stop_all) 
        {
            throw runtime_error("ThreadPool ��� ������");
        }

        using return_type = typename result_of<F(Args...)>::type; // ���ǻ� ���� ����
        auto job = make_shared<packaged_task<return_type()>>(bind(forward<F>(f), forward<Args>(args)...)); // forward : ���۷��� ������ ���� ���
        // �񵿱������� ����Ǵ� �Լ��� ���ϰ��� �޾Ƴ��� ���� packaged_task ���
        // packaged_task�� �����ڴ� �Լ� ���� �ޱ� ������ ���ڵ��� bind�Ѵ�.

        future<return_type> job_result_future = job->get_future(); // ���� �������� �����Ѵ�.
        {
            lock_guard<mutex> lock(m_job_q_); // ���ؽ� �� 
            jobs_.push([job]() { (*job)(); }); // jobs�� job�� �߰��Ѵ�.
        }

        cv_job_q_.notify_one();

        // ���� ���� ����Ǹ� ��(�Լ� f)�� ���ϰ��� job_result_future�� ���� �ǰ� �̴� Ǯ ����ڰ� ������ �� �ְԵȴ�.

        return job_result_future; 
    }

    ThreadPool::~ThreadPool() 
    {
        // ��ž�� ������ �� ��� �����带 �����.
        stop_all = true;
        cv_job_q_.notify_all();

        for (auto& t : worker_threads_) 
            t.join();
    }

}  // namespace ThreadPool





int work(int t, int id) 
{
    /*printf("%d start \n", id);
    this_thread::sleep_for(chrono::seconds(t));
    printf("%d end after %ds\n", id, t);*/

    printf("%d start \n", id);
    this_thread::sleep_for(chrono::seconds(t));
    printf("%d end\n", id);

    return t + id;
}

void ThreadPool_Test() 
{
    ThreadPool::ThreadPool pool(3); // 3���� ������ ����

    vector<future<int>> futures; // �����忡 �ο��ϴ� �½�ũ �Լ��� ��ȯ���� �����ϴ� ��츦 ����� ��ȯ���� ���� �� �ֵ��� �����Ѵ�.

    for (int i = 0; i < 10; i++)  // 3���� �����忡 ���� �� 10���� Ŀ�ǵ�(������ ���ϴ� �Լ�) �ο�
    {
        futures.emplace_back(pool.EnqueueJob(work, i % 3 + 1, i));
    }

    for (auto& f : futures) 
    {
        printf("result : %d \n", f.get());
    }
}


//ThreadPool_Test