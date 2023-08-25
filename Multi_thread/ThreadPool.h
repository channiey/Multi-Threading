#pragma once
/*

    ������ Ǯ

    ������Ʈ Ǯ�� �����ϴ�. ���� ���� ��������� ����ϰ� �ִٰ� �½�ũ�� ������ �ȴٸ�, ����ϴ� �������� �ϳ��� �̸� �޾Ƽ� �����Ѵ�.

    // �������� �浹ó���� ��� ���� -> �ھ �ִ� ������ Ȱ��

    https://modoocode.com/285

*/

#include "Defines.h"

class CThreadPool
{

public:
    CThreadPool(size_t num_threads);
    ~CThreadPool();

    template <class F, class... Args>
    future<typename result_of<F(Args...)>::type> EnqueueJob(F&& f, Args&&... args) // �½�Ʈ �߰��� ���� �Լ� ���ø�
    /*
        job �� �߰��Ѵ�.
        EnqueJob()�Լ��� ������ ������ �Լ�(�½�ũ)�� �ް� �� �Լ��� ���ϰ��� �����ϴ� feture�� �����ϵ��� �Ѵ�. feture�� ���� ���� <>�ȿ� ����.
        typename std::result_of<F(Args...)>::type  ==  f �� ���ϰ�

        ���� ���Ǵ� �Ʒ��� ����.
        template <class F, class... Args> // ���� ���� ���ø�
        std::future<f �� ���� Ÿ��> EnqueueJob(F f, Args... args); // �Լ� F�� ���ϰ��� �˱����� result_of�� ����Ѵ�.
    */
    
    {
        if (stop_all)
            throw runtime_error("ThreadPool ��� ������");

        using return_type = typename result_of<F(Args...)>::type; // ���ǻ� ���� ����
        auto job = make_shared<packaged_task<return_type()>>(bind(forward<F>(f), forward<Args>(args)...)); // forward : ���۷��� ������ ���� ���
        /*
        
            �񵿱������� ����Ǵ� �Լ��� ���ϰ��� �޾Ƴ��� ���� packaged_task ���
            packaged_task�� �����ڴ� �Լ� ���� �ޱ� ������ ���ڵ��� bind�Ѵ�.

        */

        future<return_type> job_result_future = job->get_future(); // ���� �������� �����Ѵ�.
        {
            lock_guard<mutex> lock(m_job_q_); // ���ؽ� �� 
            jobs_.push([job]() { (*job)(); }); // jobs�� job�� �߰��Ѵ�.
        }

        cv_job_q_.notify_one(); // ����ϰ� �ִ� ������ �� �ϳ��� �����.

        // ���� ���� ����Ǹ� ��(�Լ� f)�� ���ϰ��� job_result_future�� ���� �ǰ� �̴� Ǯ ����ڰ� ������ �� �ְԵȴ�.

        return job_result_future;
    }


private:
    void                        WorkerThread(); 
    /*
        thread Ŭ������ ������ �½�ũ�� �ο��ؾ� �Ѵ�.

        �� �Լ��� thread ������ �־��� �Լ��̰�, �������� �۾� �Լ��� ť�� ����ִ�.

        �� �Լ��� ť�� �ִ� �۾��� ���� �����ϴ� ������ �Ѵ�.
    
    */

private:
    size_t                      num_threads_;
    vector<thread>              worker_threads_;

    queue<function<void()>>     jobs_;          // �½�ũ ��� (��������� ó���ؾ��� �ϵ�)
    /*
        ���ϵ��� �����ϴ� job ť. -> Ŀ�ǵ� ����Ʈ (�����ؾ��� �Լ��� �����Ѵ�, function�� �Լ��� ��ü�� �����ϱ� ���� ���)
        ��� �۾� ������鿡�� ������ �����ϴ�. condition_variable�� mutex�� ��ȣ �� ���
    */

    condition_variable          cv_job_q_;      // (������ ������ ������Ǯ�� ����ϴ� ����� = �۾��� �߰��ϴ� ����� / �Һ��ڵ��� ������)
    mutex                       m_job_q_;

    bool                        stop_all;
    /*
        ��� �����带 �����ϱ� ����
        ��Ŀ ��������� �⺻������ jobs�� ó���ϴ� ���� ���ѷ����� ���� �ִµ� stop_All�� �����ȴٸ� ���ѷ����� ���������� �ȴ�.

    */
};