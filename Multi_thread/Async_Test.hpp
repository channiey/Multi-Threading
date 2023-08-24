/*
	promise�� packaged_task�� �񵿱������� �����ϱ� ���ؼ��� ��������� �����带 �����ؾ� ������,

	async�� � �Լ��� �����Ѵٸ� �ƿ� �����带 �˾Ƽ� ����� �ش� �Լ��� �񵿱������� �����ϰ� ������� future�� �����Ѵ�.
*/

int sum(const vector<int>& v, int start, int end) 
{
    int total = 0;

    for (int i = start; i < end; ++i) 
        total += v[i];

    return total;
}

int parallel_sum(const vector<int>& v) 
{
    // lower_half_future �� 1 ~ 500 ���� �񵿱������� ����
    // ����� ���� �Լ��� ����ϸ� �� �� ����ϰ� ǥ���� �� �� �ִ�.
    // --> std::async([&v]() { return sum(v, 0, v.size() / 2); });
    /*
    
        std::launch::async : �ٷ� �����带 �����ؼ� ���ڷ� ���޵� �Լ��� �����Ѵ�.

        std::launch::deferred : future �� get �Լ��� ȣ��Ǿ��� �� �����Ѵ�. (���ο� �����带 �������� ����)

        �� launch::async �ɼ��� �ָ� �ٷ� �� �ڸ����� �����带 �����ؼ� �����ϰ� �ǰ�, 
        launch::deferred �ɼ��� �ָ�, future �� get �� �Ͽ��� �� ��μ� (����������) �����ϰ� �˴ϴ�. 
        �ٽ� ����, �ش� �Լ��� ���� �ٷ� ���� �񵿱������� ������ �ʿ䰡 ���ٸ� deferred �ɼ��� �ָ� �˴ϴ�.
    
    */
    std::future<int> lower_half_future = std::async(launch::async, sum, cref(v), 0, v.size() / 2); // async()�� �Լ��� �����ϴ� �Լ��� ������� �����ϴ� future�� �����Ѵ�.

    // upper_half �� 501 ���� 1000 ���� ����
    int upper_half = sum(v, v.size() / 2, v.size());

    // lower_half_future�� upper_half�� �񵿱�� ó���ȴ�.

    return lower_half_future.get() + upper_half;
}

void Async_Test()
{
    vector<int> v;
    v.reserve(1000);

    for (int i = 0; i < 1000; ++i) 
        v.push_back(i + 1);

    cout << "1 ���� 1000 ������ �� : " << parallel_sum(v) << endl;
}