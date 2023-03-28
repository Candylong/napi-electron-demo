#include <napi.h>
#include<iostream>
#include <thread>

class MyObjWrap : public Napi::ObjectWrap<MyObjWrap> {

  public:
    static Napi::FunctionReference Initialize(Napi::Env env) {
      printf("init MyObjWrap\n");
      Napi::Function fn = DefineClass(env, "MyObjWrap", {
        InstanceMethod("print", &MyObjWrap::Print),
      });
      return Napi::Persistent(fn);
    }
    MyObjWrap(const Napi::CallbackInfo& info) : Napi::ObjectWrap<MyObjWrap>(info) {
      // ...
    }
    ~MyObjWrap() {
      // ...
    }
  
  private:
    void Print(const Napi::CallbackInfo& info){
      std::cout << "Print ..." << std::endl;
    }

};

class MyObjWrap1 : public Napi::ObjectWrap<MyObjWrap1> {

  public:
    static Napi::FunctionReference Initialize(Napi::Env env) {
      Napi::Function fn = DefineClass(env, "MyObjWrap1", {
        InstanceMethod("print1", &MyObjWrap1::Print1),
      });
      return Napi::Persistent(fn);
    }
    MyObjWrap1(const Napi::CallbackInfo& info) : Napi::ObjectWrap<MyObjWrap1>(info) {
      // ...
    }
    ~MyObjWrap1() {
      // ...
    }
  
  private:
    void Print1(const Napi::CallbackInfo& info){
      std::cout << "Print1 ..." << std::endl;
    }

};


Napi::ThreadSafeFunction myThreadFunction;
using callJsfuntion = std::function<void(Napi::Env, Napi::Function)>;
bool jsCallBack(callJsfuntion fucntion)
{
  if (napi_status::napi_ok == myThreadFunction.Acquire())
  {
      bool ok = napi_ok == myThreadFunction.BlockingCall(fucntion);
      myThreadFunction.Release();
      return ok;
  }
  else
  {
    return true;
  }
}

bool isStop(true);
void threadLoop()
{
  while(!isStop)
  {
    auto callback = [=](Napi::Env env, Napi::Function function)
    {
      Napi::Object videoFrame = Napi::Object::New(env);
      videoFrame.Set("width", Napi::Number::New(env, 1920));
      for(int i = 0; i<7000; i++)   // Compare to internal frequent tasks
      {
        videoFrame.Set(std::to_string(i).c_str(), Napi::String::New(env, "123456789"));
      }

      for(int i = 0; i<7000; i++)
      {
        videoFrame.Set(std::to_string(i).c_str(), Napi::Number::New(env, 1080));
        videoFrame.Set(std::to_string(i).c_str(), Napi::Number::New(env, 1920));
      }
      function.Call({Napi::String::New(env, "hello from thread loop 1"), videoFrame});
    };
    jsCallBack(callback);
    // sleep
  std::this_thread::sleep_for(std::chrono::microseconds(1));
  }
}

void threadLoop2()
{
  while(!isStop)
  {
    auto callback = [=](Napi::Env env, Napi::Function function)
    {
		  Napi::Object videoFrame = Napi::Object::New(env);
      videoFrame.Set("width", Napi::Number::New(env, 1920));

      for(int i = 0; i<8000; i++)
      {
        videoFrame.Set(std::to_string(i).c_str(), Napi::Number::New(env, 1080));
        videoFrame.Set(std::to_string(i).c_str(), Napi::Number::New(env, 1920));
      }

      for(int i = 0; i<8000; i++)   // Compare to internal frequent tasks
      {
        videoFrame.Set(std::to_string(i).c_str(), Napi::String::New(env, "123456789"));
      }
      function.Call({Napi::String::New(env, "hello from thread loop 2"), videoFrame});
    };
    jsCallBack(callback);
    // sleep
  std::this_thread::sleep_for(std::chrono::microseconds(1));
  }
}

class MyObjWrap2 : public Napi::ObjectWrap<MyObjWrap2> {

  public:
    static Napi::FunctionReference Initialize(Napi::Env env) {
      printf("init MyObjWrap2\n");
      Napi::Function fn = DefineClass(env, "MyObjWrap2", {
        InstanceMethod("print2", &MyObjWrap2::Print2),
      });
      return Napi::Persistent(fn);
    }
    MyObjWrap2(const Napi::CallbackInfo& info) : Napi::ObjectWrap<MyObjWrap2>(info) {
      // ...
      isStop = false;
      // thread 1
      mythread1 = std::thread(std::bind(threadLoop));
      // thread 2
      mythread2 = std::thread(std::bind(threadLoop2));
    }
    ~MyObjWrap2() 
    {
      isStop = true;
      mythread1.join();
      mythread2.join();
    }
  
  private:
    void Print2(const Napi::CallbackInfo& info){
      std::cout << "Print2 ..." << std::endl;
    }
    std::thread mythread1;
    std::thread mythread2;

};


class CustomAddon : public Napi::Addon<CustomAddon> {
 public:

  Napi::Value CreateFunction(const Napi::CallbackInfo& info)
  {
    printf("CustomAddon Function \n");
    Napi::Env env = info.Env();
    Napi::Object param = info[0].As<Napi::Object>();
    if (!param.Has("tsfn"))
    {
        Napi::TypeError::New(env, "Wrong arguments").ThrowAsJavaScriptException();
        return env.Undefined();
    }

    Napi::Function tsfn = param.Get("tsfn").As<Napi::Function>();
    myThreadFunction = Napi::ThreadSafeFunction::New(
                    env,
                    tsfn,            // JavaScript function called asynchronously
                    "Resource Name", // Name
                    0,               // Unlimited queue
                    1,               // Only one thread will use this initially
                    [](Napi::Env) {  // Finalizer used to clean threads up
                        printf("Addon tsfn finalizer \n");
                    });

    env.AddCleanupHook([=](){
      printf("execute clean hook\n");
      myThreadFunction.Release();
    });
    return Napi::Number::New(env, 0);
  }

  Napi::Value CreateObject(const Napi::CallbackInfo& info)
  {
    printf("CustomAddon CreateObject \n");
    Napi::Env env = info.Env();

    Napi::Object jsObject = myObjWrapCtr2.New({});
    return jsObject;
  }
  
  CustomAddon(Napi::Env env, Napi::Object exports) {
    myObjWrapCtr = MyObjWrap::Initialize(env);
    myObjWrapCtr1 = MyObjWrap1::Initialize(env);
    myObjWrapCtr2 = MyObjWrap2::Initialize(env);
    DefineAddon(exports, {
    InstanceMethod("CreateFunction", &CustomAddon::CreateFunction),
    InstanceMethod("CreateObject", &CustomAddon::CreateObject)}
    );
  }
 private:
  Napi::FunctionReference myObjWrapCtr;
  Napi::FunctionReference myObjWrapCtr1;
  Napi::FunctionReference myObjWrapCtr2;
};

// The macro announces that instances of the class `CallasAddon` will be
// created for each instance of the add-on that must be loaded into Node.js.
//NODE_API_NAMED_ADDON(NODE_GYP_MODULE_NAME, Addon)
NODE_API_ADDON(CustomAddon)