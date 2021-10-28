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
