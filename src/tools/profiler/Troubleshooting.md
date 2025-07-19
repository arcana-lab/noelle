# Troubleshooting

1. **"unknown special variable" error**: Use `-profiler-no-auto-cleanup` flag
2. **Too many functions instrumented**: Use `-profiler-ignore` flag
3. **Linking errors**: Make sure to link with `install/lib/libprofiler.so`
4. **No timing output**: Check if cleanup function is being called
