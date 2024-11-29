set listsize 30
handle SIGSEGV nostop print pass
dir ../src

define showinput
p ctx->input.buffer+ctx->input.offset
end

define showds
p ctx->ds.top + 1 - ctx->ds.base
p ctx->ds
x/16gx ctx->ds.base-2
end

define showfs
p ctx->fs.top + 1 - ctx->fs.base
p ctx->fs
x/8gx ctx->fs.base-2
end

define showrs
p ctx->rs.top + 1 - ctx->rs.base
p ctx->rs
x/24gx ctx->rs.base-2
end

define showword
p ctx->words
p *ctx->words
x/8gx ctx->words->data
end

define showwords
  set var $p = *ctx->active
  while $p != 0
    printf "%#lx %s\n", $p, $p->name
    set var $p = $p->prev
  end
end

set can-use-hw-watchpoints 0

show user