# agrpc

Build an elegant GRPC Async interface with C++20 coroutine and libunifex (target for C++23 executor).

## Get started
```bash
mkdir build && cd build
conan install ..
cmake --build .
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

## Work in progress
...
