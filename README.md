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

## Benchmark

### 2 CPU server

| name                        |   req/s |   avg. latency |        90 % in |        95 % in |        99 % in | avg. cpu |   avg. memory |
|-----------------------------|--------:|---------------:|---------------:|---------------:|---------------:|---------:|--------------:|
| rust_tonic_mt               |   77236 |       11.97 ms |       30.99 ms |       44.78 ms |       68.00 ms |  191.33% |     17.42 MiB |
| cpp_agrpc                   |   72084 |       13.50 ms |       20.83 ms |       23.73 ms |       31.08 ms |  202.99% |      86.8 MiB |
| cpp_grpc_mt                 |   68099 |       14.18 ms |       34.95 ms |       39.29 ms |       45.88 ms |  203.08% |     61.64 MiB |
| dotnet_grpc                 |   63479 |       13.98 ms |       20.35 ms |       26.62 ms |       61.63 ms |  201.58% |    175.97 MiB |
| rust_tonic_st               |   61962 |       15.83 ms |       19.73 ms |       21.05 ms |       24.13 ms |  102.46% |     16.32 MiB |
| cpp_grpc_st                 |   50593 |       19.47 ms |       23.59 ms |       25.26 ms |       29.66 ms |  102.15% |     19.53 MiB |
| cpp_grpc_callback           |   29612 |       32.49 ms |       74.33 ms |       78.51 ms |       85.74 ms |  202.37% |    444.44 MiB |
| go_grpc                     |   12050 |       80.52 ms |      188.05 ms |      194.19 ms |      286.45 ms |  200.78% |     45.11 MiB |


## Goal
Maybe something like C# interface.
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
