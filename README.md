# agrpc

Build an elegant GRPC async interface with C++20 coroutine and libunifex (target for C++23 executor).

## Get started
```bash
mkdir build && cd build
conan install ..
cmake -DCMAKE_CXX_FLAGS:STRING=-fcoroutines ..
cmake --build .
```

## Work in progress
One glance:
<table>
<tr>
<td> native async grpc </td> <td> agrpc </td>
</tr>
<tr>
<td>

```c++
class CallData {
 public:
  void Proceed() {
    if (status_ == CREATE) {
      status_ = PROCESS;
      service_->RequestSayHello(&ctx_, &request_,
                                &responder_,
                                cq_, cq_, this);
    } else if (status_ == PROCESS) {
      new CallData(service_, cq_);
  
      std::string prefix("Hello ");
      reply_.set_message(prefix + request_.name());

      status_ = FINISH;
      responder_.Finish(reply_, Status::OK, this);
    } else {
      GPR_ASSERT(status_ == FINISH);
      delete this;
    }
  }

 private:
  // ...
  enum CallStatus { CREATE, PROCESS, FINISH };
  CallStatus status_{CREATE};  // The current serving state.
};

void HandleRpcs() {
  new CallData(&service_, cq_.get());
  void* tag;  // uniquely identifies a request.
  bool ok;
  while (true) {
    GPR_ASSERT(cq_->Next(&tag, &ok));
    GPR_ASSERT(ok);
    static_cast<CallData*>(tag)->Proceed();
  }
}
```

</td>
<td>
    
```c++
while (true) {
  grpc::ServerContext server_context;
  helloworld::HelloRequest request;
  grpc::ServerAsyncResponseWriter<helloworld::HelloReply> writer{
      &server_context};
  bool request_ok = co_await agrpc::AsyncRequest(
      grpc_context.get_scheduler(),
      &helloworld::Greeter::AsyncService::RequestSayHello,
      service, server_context, request, writer);
  if (!request_ok)
    co_return;
  helloworld::HelloReply response;
  response.set_message("Hello " + request.name());
  co_await agrpc::AsyncFinish(grpc_context.get_scheduler(), writer,
                              response, grpc::Status::OK);
}
```
</td>
</tr>
</table>

## Goal
Maybe somethiing like C# interface.
```c#
public override async Task RouteChat(Grpc.Core.IAsyncStreamReader<RouteNote> requestStream,
    Grpc.Core.IServerStreamWriter<RouteNote> responseStream,
    Grpc.Core.ServerCallContext context)
{
    while (await requestStream.MoveNext())
    {
        var note = requestStream.Current;
        List<RouteNote> prevNotes = AddNoteForLocation(note.Location, note);
        foreach (var prevNote in prevNotes)
        {
            await responseStream.WriteAsync(prevNote);
        }
    }
}
```

## Reference
- [asio-grpc: Asynchronous gRPC with Boost.Asio executors](https://github.com/Tradias/asio-grpc)
