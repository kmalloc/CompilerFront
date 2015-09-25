#include "gtest/gtest.h"

#include "ThreadPool.h"

using namespace xthread;

TEST(test_xthread, test_basic_thread)
{
    ThreadPool pool(4);

    bool f1 = false, f2 = false;
    std::mutex m1, m2;
    std::condition_variable cv1, cv2;

    std::atomic<int> i1{1}, i2{1}, s{0};

    auto dt = [&]() { s++; };

    for (auto i = 0; i < 1024; ++i)
    {
        pool.PushTask(dt);
    }

    std::vector<size_t> num = pool.GetTaskNum();

    ASSERT_EQ(4, num.size());
    ASSERT_EQ(256, num[0]);
    ASSERT_EQ(256, num[1]);
    ASSERT_EQ(256, num[2]);
    ASSERT_EQ(256, num[3]);

    pool.StartWorking();

    pool.PushTask([&]() { i1 = 233; std::unique_lock<std::mutex> lock{m1}; f1 = true; cv1.notify_one();});
    pool.PushTask([&]() { i2 = 234; std::unique_lock<std::mutex> lock{m2}; f2 = true; cv2.notify_one();});

    {
        std::unique_lock<std::mutex> lock{m1};
        while (!f1) cv1.wait(lock);

        ASSERT_EQ(233, i1);
    }

    {
        std::unique_lock<std::mutex> lock{m2};
        while (!f2) cv2.wait(lock);

        ASSERT_EQ(234, i2);
    }

    pool.CloseThread(true);
    pool.Shutdown();

    ASSERT_EQ(1024, s);
}

TEST(test_xthread, test_exiting_pool)
{
    ThreadPool pool(4);

    pool.StartWorking();

    bool f1 = false, f2 = false, f3 = false, f4 = false;
    std::mutex m, m1, m2, m3, m4;
    std::atomic<int> v1{0}, v2{0}, v3{0}, v4{0}, s{0};
    std::condition_variable cv, cv1, cv2, cv3, cv4;

    auto bt1 = [&]() { s++; cv.notify_one(); std::unique_lock<std::mutex> lock{m1}; while (!f1) cv1.wait(lock); };
    auto bt2 = [&]() { s++; cv.notify_one(); std::unique_lock<std::mutex> lock{m2}; while (!f2) cv2.wait(lock); };
    auto bt3 = [&]() { s++; cv.notify_one(); std::unique_lock<std::mutex> lock{m3}; while (!f3) cv3.wait(lock); };
    auto bt4 = [&]() { s++; cv.notify_one(); std::unique_lock<std::mutex> lock{m4}; while (!f4) cv4.wait(lock); };

    auto rt1 = [&]() { v1 = 10; };
    auto rt2 = [&]() { v2 = 20; };
    auto rt3 = [&]() { v3 = 30; };
    auto rt4 = [&]() { v4 = 40; };

    pool.PushTask([&]() { std::cout << "dummy lambda.\n"; });
    pool.PushTask(bt1);
    pool.PushTask(bt2);
    pool.PushTask(bt3);
    pool.PushTask(bt4);

    // following 4 task should not have a chance to run.
    pool.PushTask(rt1);
    pool.PushTask(rt2);
    pool.PushTask(rt3);
    pool.PushTask(rt4);

    // make sure all the threads are running.
    {
        std::unique_lock<std::mutex> lock{m};
        while (s != 4) cv.wait(lock);
    }

    // cancel all the threads immediately.
    pool.CloseThread(false);

    {
        std::unique_lock<std::mutex> lock{m1};
        f1 = true;
    }
    cv1.notify_one();

    {
        std::unique_lock<std::mutex> lock{m2};
        f2 = true;
    }
    cv2.notify_one();

    {
        std::unique_lock<std::mutex> lock{m3};
        f3 = true;
    }
    cv3.notify_one();

    {
        std::unique_lock<std::mutex> lock{m4};
        f4 = true;
    }
    cv4.notify_one();

    pool.Shutdown();

    ASSERT_EQ(0, v1);
    ASSERT_EQ(0, v2);
    ASSERT_EQ(0, v3);
    ASSERT_EQ(0, v4);

    // test gracefully shutdown.
    f1 = f2 = f3 = f4 = false;
    pool.StartWorking();

    pool.PushTask(bt1);
    pool.PushTask(bt2);
    pool.PushTask(bt3);
    pool.PushTask(bt4);

    pool.PushTask(rt1);
    pool.PushTask(rt2);
    pool.PushTask(rt3);
    pool.PushTask(rt4);

    // shutdown all the threads after the pending tasks are run.
    pool.CloseThread(true);

    {
        std::unique_lock<std::mutex> lock{m1};
        f1 = true;
    }
    cv1.notify_one();

    {
        std::unique_lock<std::mutex> lock{m2};
        f2 = true;
    }
    cv2.notify_one();

    {
        std::unique_lock<std::mutex> lock{m3};
        f3 = true;
    }
    cv3.notify_one();

    {
        std::unique_lock<std::mutex> lock{m4};
        f4 = true;
    }
    cv4.notify_one();

    pool.Shutdown();

    ASSERT_EQ(10, v1);
    ASSERT_EQ(20, v2);
    ASSERT_EQ(30, v3);
    ASSERT_EQ(40, v4);
}

TEST(test_xthread, performance_test)
{
    ThreadPool pool;
    pool.StartWorking();

    auto t1 = [&]() { int i = 0; while (i < 100000) { ++i; } };
    auto t2 = [&]() { int i = 0; while (i < 100000) { ++i; } };

    std::mutex m1, m2, m3;
    bool f1 = false, f2 = false, f3 = false;

    auto th1 = [&]()
    {
        while (1)
        {
            pool.PushTask(t1);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            std::unique_lock<std::mutex> lock{m1};
            if (f1) break;
        }
    };

    auto th2 = [&]()
    {
        while (1)
        {
            pool.PushTask(t2);
            std::this_thread::yield();
            // std::this_thread::sleep_for(std::chrono::milliseconds(10));
            std::unique_lock<std::mutex> lock{m2};
            if (f2) break;
        }
    };

    auto th3 = [&]()
    {
        while (1)
        {
            pool.PushTask(t2);
            std::this_thread::yield();
            // std::this_thread::sleep_for(std::chrono::milliseconds(10));
            std::unique_lock<std::mutex> lock{m3};
            if (f3) break;
        }
    };

    std::thread thread1(th1), thread2(th2), thread3(th3);

    std::this_thread::sleep_for(std::chrono::seconds(2));

    {
        std::unique_lock<std::mutex> lock{m1};
        f1 = true;
    }

    {
        std::unique_lock<std::mutex> lock{m2};
        f2 = true;
    }

    {
        std::unique_lock<std::mutex> lock{m3};
        f3 = true;
    }

    thread1.join();
    thread2.join();
    thread3.join();

    std::cout << "\nnow waitting for all the tasks to finish, there are "
              << pool.GetThreadNum() << " workers working on it, "
              << "but it is still going to take long.\n" << std::endl;

    while (1)
    {
        int all = 0;
        auto num = pool.GetTaskNum();
        for (auto i: num)
        {
            all += i;
        }

        if (!all) break;

        std::cout << "(" << all << ") tasks remaining." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(400));
    }

    pool.CloseThread(true);
}

