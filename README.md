# agrpc

Build an elegant GRPC async interface with C++20 coroutine and libunifex (target for C++23 executor).

## Get started
```bash
mkdir build && cd build
conan install ..
cmake --build .
```

## Work in progress
One glance:
```c++
while (true) {
  grpc::ServerContext server_context;
  grpc::ServerAsyncResponseWriter<helloworld::HelloReply> writer{&server_context};
  auto [request, request_ok] = co_await agrpc::GrpcContext::RequestSender(
    &helloworld::Greeter::AsyncService::RequestSayHello, grpc_context,
    service, server_context, writer);

  if (!request_ok)
    co_return;
  }
```

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
