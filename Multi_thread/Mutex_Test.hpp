/*
	Mutex를 이용한 일반 멀티 쓰레딩

	https://modoocode.com/270
*/

#include "Defines.h"

void Mutex_Test() 
{
	mutex mtx; // 스레드간 공유하는 뮤텍스 객체 (스레드 동기화를 위해 사용)

	int iCnt = 0; // 디버깅 카운트
	int iSum = 0; // 스레드간 공유할 메모리

	const int		iNumOfThread = 3;
	vector<thread>	vecThread;

	vecThread.reserve(iNumOfThread);

	// 스레드 태스크
	auto work_func = [&](const string& name, int& _iSum) // 스레드간 공유할 메모리를 인자로 넘길지 클래스가 갖고있을지는 경우에 따라 선택
	{
		for (int i = 0; i < 5; ++i)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));

			// 어떤 스레드가 작업중이라면 다른 스레드가 접근하지 못하도록 잠근다.
			mtx.lock();

			// lock()과 unlock() 사이의 코드는 하나의 스레드만 접근하도록 한다 - 공유되는 메모리 사용 영역. -> 임계영역 설정 Critical Section (이것을 위해 mutex를 사용한다)
			/*
				this_thread = 현재 실행중인 스레드를 의미한다.
			*/

			{
				++_iSum; // 공유 메모리에 접근하여 쓰기를 수행한다.

				cout << name << "-thread(" << std::this_thread::get_id() << ")is working " << i << endl;
				if (0 == (++iCnt % iNumOfThread)) cout << endl;

			}

			// 스레드의 작업이 끝났다면 다시 잠금을 해제한다.
			/*
				해제하지 않을 경우 무한정 기다린다 -> DeadLock(본인도 다시 lock을 호출해야하는데 본인이 unlock하지 않아 접근 불가)

				데드락을 해결하는 것은 복잡하다. 데드락이 발생하지 않도록 프로그램을 잘 설계하는 것이 중요하다.
			*/
			mtx.unlock();

			// 지역 변수 특성을 사용하여 lock_guard() 함수로 편리하게 락 언락을 가능케 할 수도 있다.
			/*
				https://modoocode.com/270
			*/
		}
	};

	// 스레드 작업 시작
	cout << "Thread Working Start\n\n";

	// 메인 스레드에서 A라는 키워드로 자식 스레드를 생성하고 태스크 부여,  그 후 메인스레드는 바로 아래 라인으로 진행 (자식 스레드는 자기 태스크 진행)
	// 스레드에 참조로 인자를 전달할 경우 std::ref를 사용해야 한다. -> 전달받은 인자를 복사가능한 레퍼런스로 변환한다. (cosnt 레퍼런스 일경우 cref 호출)
	vecThread.push_back(std::thread(work_func, "A", std::ref(iSum)));
	vecThread.push_back(std::thread(work_func, "B", std::ref(iSum)));
	vecThread.push_back(std::thread(work_func, "C", std::ref(iSum)));

	// 해당 스레드가 종료될 때까지 호출 스레드의 실행을 stop하는 함수
	/*
		t1.join(); 이라면 호출스레드(여기서는 메인스레드)는 t1스레드가 실행이 끝날때까지 멈추게 되고, t1스레드의 실행이 종료되면 메인스레드가 다시 실행된다.

		부모 스레드에 의해 갈라진 자식 스레드가 실행이 종료되어 부모 스레드에 join한다는 식으로 생각

		join이 존재하지 않는다면, 자식 스레드가 실행중인데도 부모 스레드가 저 아래에 있는 return 0을 호출하여 프로그램이 종료되는 상황이 발생할 수 있기 때문이다.

		기본적으로 메인스레드가 종료되고도 자식스레드가 계속 실행중이라면 컴파일러는 비정상오류로 인식하고 오류를 발생시킨다.

		만약 이것이 의도된 것이라면 detatch()함수를 사용해야 한다.

	*/
	for (int i = 0; i < vecThread.size(); ++i)
		vecThread[i].join();

	// 스레드 작업 종료
	cout << "Thread Working Finish\n";
	cout << "Sum : " << iSum << endl;

	// 추가 : Detach() 함수
	/*

		부모 스레드는 자식 스레드의 실행 또는 종료를 신경쓰지 않고 계속 진행이가능하지만, 자식 스레드가 끝나기 전에 부모 스레드가 먼저 종료되어 프로그램이 종료된다면,
		운영체제는 비정상적인 행동이라고 인지하여 오류를 발생시킨다. 이때 이것을 의도한 것이라고 운영체제에게 알려주는 함수가 이 함수이다.

		내부적으로 스레드의 id를 0으로 만들어 스레드의 소멸자가 불려도 오류를 발생시키지 않는다. (스레드 ID가 0이라는 것은 현재 스레드가 비활성화 상태임을 나타낸다.)

	*/
}