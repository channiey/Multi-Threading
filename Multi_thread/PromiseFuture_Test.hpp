/*
    �񵿱��� ������ �ٷ��. -> ��� �����͸� �ٸ� �����带 ���� ó���ؼ� �޾Ƴ���.

    ���� � ������ T�� ����ؼ� �񵿱������� ���� �޾Ƴ��ڴ� -> �̷�(Future)�� ������ T�� ���ϴ� �����͸� �����ְڴٴ� ���(Promise)

    promise ��ü�� furture ��ü�� ���� �ִ�.

    ������ �Һ��� ���ҿ��� promise�� ������, future�� �Һ����� ������ �����Ѵٰ� �� �� �ִ�.

    ������ �Һ��� ���� ��� promise future�� ����ϴ� ���� �������� ���� future�� ���ܵ� ������ �� �ֱ� �����̴�. ���� ���Ѵ�.

    https://modoocode.com/284
*/

#include "Defines.h"


void Worker(promise<string>* p)
{
    // Promise ��ü�� �ڽ��� ������ �ִ� future ��ü�� ���� �־��ش�.
    p->set_value("Processed Data");
}

void PromiseFuture_Test()
{
    promise<string> p; 
    // promise ���� : ���� ������ ������ ��ü�� Ÿ���� ���ø� ���ڷ� �޴´�.
    // ������ ���� ���� promise ��ü�� �ڽ��� ������ �ִ� future ��ü�� ���� �־��ְ� �ȴ�.

    future<string> data = p.get_future();  // promise ��ü�� �����Ǵ� future ��ü�� ���� ��Ƶд�.

    thread t(Worker, &p);

    data.wait(); // �̷��� ��ӵ� �����͸� ���� ������ ��ٸ���. (�ٵ� ���� wait �Ƚᵵ ����� ����)

    // wait�� ���ϵǾ��ٴ� ���� future�� �����Ͱ� ���õǾ��ٴ� �ǹ�
    // wait���� �׳� get�ص� wait �� �Ͱ� ����.
    // future���� get�� ȣ���ϸ�, ������ ��ü�� �̵��ȴ�. ���� ���� get�� �ٽ� �ѹ� �� ȣ���ϸ� �ȵȴ�.
    cout << "Received Data : " << data.get() << endl;

    t.join();
}