public class BitonicSort{
    public static void main(String[] args) {
        int p = Integer.parseInt(args[0]);
        int granularity = Integer.parseInt(args[1]);
        int n = 1 << 22;  // N = 2^22
        double seconds = 10.0;

        SortThread sorter = new SortThread(0, n);

        int count = 0;
        long start = System.currentTimeMillis();
        long end = start + (long)(seconds * 1000);

        while (System.currentTimeMillis() < end) {
            // TODO: fill array with random data
            sorter.fillWithRandomNum(n);
            // TODO: sort the array
            sorter.run();
            // TODO: (optional) verify it's sorted
            count++;
        }

        long elapsed = System.currentTimeMillis() - start;
        System.out.println("Sorted " + count + " arrays (each: " + n + " elements) in " +
                elapsed + " ms using " + p + " threads");
    }
}

