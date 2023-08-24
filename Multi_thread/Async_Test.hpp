/*
	promise나 packaged_task를 비동기적으로 실행하기 위해서는 명시적으로 쓰레드를 생성해야 했지만,

	async에 어떤 함수를 전달한다면 아예 스레드를 알아서 만들어 해당 함수를 비동기적으로 실행하고 결과값을 future에 전달한다.
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
    // lower_half_future 는 1 ~ 500 까지 비동기적으로 더함
    // 참고로 람다 함수를 사용하면 좀 더 깔끔하게 표현할 수 도 있다.
    // --> std::async([&v]() { return sum(v, 0, v.size() / 2); });
    /*
    
        std::launch::async : 바로 쓰레드를 생성해서 인자로 전달된 함수를 실행한다.

        std::launch::deferred : future 의 get 함수가 호출되었을 때 실행한다. (새로운 쓰레드를 생성하지 않음)

        즉 launch::async 옵션을 주면 바로 그 자리에서 쓰레드를 생성해서 실행하게 되고, 
        launch::deferred 옵션을 주면, future 의 get 을 하였을 때 비로소 (동기적으로) 실행하게 됩니다. 
        다시 말해, 해당 함수를 굳이 바로 당장 비동기적으로 실행할 필요가 없다면 deferred 옵션을 주면 됩니다.
    
    */
    std::future<int> lower_half_future = std::async(launch::async, sum, cref(v), 0, v.size() / 2); // async()는 함수를 실행하는 함수의 결과값을 포함하는 future를 리턴한다.

    // upper_half 는 501 부터 1000 까지 더함
    int upper_half = sum(v, v.size() / 2, v.size());

    // lower_half_future와 upper_half가 비동기로 처리된다.

    return lower_half_future.get() + upper_half;
}

void Async_Test()
{
    vector<int> v;
    v.reserve(1000);

    for (int i = 0; i < 1000; ++i) 
        v.push_back(i + 1);

    cout << "1 부터 1000 까지의 합 : " << parallel_sum(v) << endl;
}