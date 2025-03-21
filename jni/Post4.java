/*
 * Post4.java
 *
 * Copyright 2023, 2024 by Anthony Howe.  All rights reserved.
 */

package post4.jni;

public class Post4 implements AutoCloseable
{
	private long ctx;

	static
	{
		System.loadLibrary("post4jni");
		p4Init();
	}

	public Post4()
	{
		this(new Post4Options());
	}

	public Post4(String[] args)
	{
		Post4Options opts = new Post4Options();
		opts.argc = args.length;
		opts.argv = args;
		ctx = p4Create(opts);
	}

	public Post4(Post4Options opts)
	{
		ctx = p4Create(opts);
	}

	@Override
	public void close()
	{
			p4Free(ctx);
			ctx = 0;
	}

	public static void main(String[] args)
	{
		Post4 p4 = new Post4(args);

		if (0 < args.length) {
			try {
				p4.evalFile(args[0]);
				System.exit(0);
			} catch (Exception e) {
				// Something else happened on this day, lost in time.
				System.err.println(e);
				System.exit(1);
			}
		}

		try {
			// Quick basic sanity test.
			p4.evalString(
"""
$cafebabe HEX U. DECIMAL
.( Monaco ) 377 .
.( e ) 2.71828 F.
.( PI ) 3.14159 F.
.( Planck's ) 6.62607015e-34 FS. CR
"""
			);

			p4.repl();
		} catch (Exception e) {
			// Something else happened on this day, lost in time.
			System.err.println(e);
		}
	}

	private native static void p4Init();
	private native static void p4Free(long ctx);
	private native static long p4Create(Post4Options opts);

	public native int repl();
	public native Post4Stacks stacks();
	public native void evalFile(String fpath) throws Post4Exception;
	public native void evalString(String string) throws Post4Exception;
}
