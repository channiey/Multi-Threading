/*
    비동기적 실행을 다룬다. -> 어떠한 데이터를 다른 쓰레드를 통해 처리해서 받아낸다.

    내가 어떤 쓰레드 T를 사용해서 비동기적으로 값을 받아내겠다 -> 미래(Future)에 쓰레드 T가 원하는 데이터를 돌려주겠다는 약속(Promise)

    promise 객체가 furture 객체를 갖고 있다.

    생산자 소비자 역할에서 promise가 생산자, future는 소비자의 역할을 수행한다고 볼 수 있다.

    생산자 소비자 패턴 대신 promise future를 사용하는 것은 가독성이 좋고 future에 예외도 전달할 수 있기 때문이다. 또한 편한다.

    https://modoocode.com/284
*/

#include "Defines.h"


void Worker(promise<string>* p)
{
    // Promise 객체가 자신이 가지고 있는 future 객체에 값을 넣어준다.
    p->set_value("Processed Data");
}

void PromiseFuture_Test()
{
    promise<string> p; 
    // promise 정의 : 연산 수행후 돌려줄 객체의 타입을 템플릿 인자로 받는다.
    // 연산이 끝난 다음 promise 객체는 자신이 가지고 있는 future 객체에 값을 넣어주게 된다.

    future<string> data = p.get_future();  // promise 객체에 대응되는 future 객체를 따로 담아둔다.

    thread t(Worker, &p);

    data.wait(); // 미래에 약속된 데이터를 받을 때까지 기다린다. (근데 굳이 wait 안써도 결과는 같다)

    // wait이 리턴되었다는 것은 future에 데이터가 세팅되었다는 의미
    // wait없이 그냥 get해도 wait 한 것과 같다.
    // future에서 get을 호출하면, 설정된 객체가 이동된다. 따라서 절대 get을 다시 한번 더 호출하면 안된다.
    cout << "Received Data : " << data.get() << endl;

    t.join();
}