public static void main(String[] args) {
    int p = Integer.parseInt(args[0]);
    int granularity = Integer.parseInt(args[1]);
    int n = 1 << 22;  // N = 2^22
    double seconds = 10.0;

    // TODO: setup your sorter with n, p, and granularity

    int count = 0;
    long start = System.currentTimeMillis();
    long end = start + (long)(seconds * 1000);

    while (System.currentTimeMillis() < end) {
        // TODO: fill array with random data
        // TODO: sort the array
        // TODO: (optional) verify it's sorted
        count++;
    }

    long elapsed = System.currentTimeMillis() - start;
    System.out.println("Sorted " + count + " arrays (each: " + n + " elements) in " +
            elapsed + " ms using " + p + " threads");
}