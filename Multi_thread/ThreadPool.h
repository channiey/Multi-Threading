#pragma once
/*

    쓰레드 풀

    오브젝트 풀과 유사하다. 여러 개의 쓰레드들이 대기하고 있다가 태스크가 들어오게 된다면, 대기하던 쓰레드중 하나가 이를 받아서 실행한다.

    // 렌더링과 충돌처리에 사용 가능 -> 코어를 최대 개수로 활용

    https://modoocode.com/285

*/

#include "Defines.h"

class CThreadPool
{

public:
    CThreadPool(size_t num_threads);
    ~CThreadPool();

    template <class F, class... Args>
    future<typename result_of<F(Args...)>::type> EnqueueJob(F&& f, Args&&... args) // 태스트 추가를 위한 함수 템플릿
    /*
        job 을 추가한다.
        EnqueJob()함수가 임의의 형태의 함수(태스크)를 받고 그 함수의 리턴값을 보관하는 feture를 리턴하도록 한다. feture의 리턴 값이 <>안에 들어간다.
        typename std::result_of<F(Args...)>::type  ==  f 의 리턴값

        위의 정의는 아래와 같다.
        template <class F, class... Args> // 가변 길이 템플릿
        std::future<f 의 리턴 타입> EnqueueJob(F f, Args... args); // 함수 F의 리턴값을 알기위해 result_of를 사용한다.
    */
    
    {
        if (stop_all)
            throw runtime_error("ThreadPool 사용 중지됨");

        using return_type = typename result_of<F(Args...)>::type; // 편의상 따로 정의
        auto job = make_shared<packaged_task<return_type()>>(bind(forward<F>(f), forward<Args>(args)...)); // forward : 레퍼런스 전달을 위해 사용
        /*
        
            비동기적으로 실행되는 함수의 리턴값을 받아내기 위해 packaged_task 사용
            packaged_task의 생성자는 함수 만을 받기 때문에 인자들을 bind한다.

        */

        future<return_type> job_result_future = job->get_future(); // 잡의 실행결과를 보관한다.
        {
            lock_guard<mutex> lock(m_job_q_); // 뮤텍스 락 
            jobs_.push([job]() { (*job)(); }); // jobs에 job를 추가한다.
        }

        cv_job_q_.notify_one(); // 대기하고 있는 쓰레드 중 하나를 깨운다.

        // 이후 잡이 실행되면 잡(함수 f)의 리턴값이 job_result_future에 들어가게 되고 이는 풀 사용자가 접근할 수 있게된다.

        return job_result_future;
    }


private:
    void                        WorkerThread(); 
    /*
        thread 클래스는 생성시 태스크를 부여해야 한다.

        이 함수는 thread 생성시 넣어줄 함수이고, 실질적인 작업 함수는 큐에 들어있다.

        이 함수는 큐에 있는 작업을 꺼내 수행하는 역할을 한다.
    
    */

private:
    size_t                      num_threads_;
    vector<thread>              worker_threads_;

    queue<function<void()>>     jobs_;          // 태스크 목록 (쓰레드들이 처리해야할 일들)
    /*
        할일들을 보관하는 job 큐. -> 커맨드 리스트 (실행해야할 함수를 보관한다, function는 함수를 객체로 보관하기 위해 사용)
        모든 작업 쓰레드들에서 접근이 가능하다. condition_variable와 mutex로 보호 및 사용
    */

    condition_variable          cv_job_q_;      // (생산자 역할은 쓰레드풀을 사용하는 사람들 = 작업을 추가하는 사람들 / 소비자들은 쓰레드)
    mutex                       m_job_q_;

    bool                        stop_all;
    /*
        모든 쓰레드를 종료하기 위함
        워커 쓰레드들은 기본적으로 jobs를 처리하는 동안 무한루프를 돌고 있는데 stop_All이 설정된다면 무한루프를 빠져나가게 된다.

    */
};