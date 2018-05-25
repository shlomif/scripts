description = "Exports sysdig span tag durations";
short_description = "Export span tag durations.";
category = "Tracers";

args = {}

function on_init()
    ftags = chisel.request_field("span.tags")
    flatency = chisel.request_field("span.duration")
    chisel.set_filter("evt.type=tracer and evt.dir=<")
    return true
end

function on_event()
    local tags = evt.field(ftags)
    local latency = evt.field(flatency)
    if latency then
        print(tostring(tags) .. "\t" .. tonumber(latency) / 1e9)
    end
    return true
end
