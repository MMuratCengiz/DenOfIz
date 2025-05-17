package com.denofiz.graphics;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.function.Supplier;

/**
 * A very simple pool implementation, this is simply for demonstration purposes when the amount of objects
 * you will use are known beforehand, even better use static classes, but this is here if you'd like to keep LOC short.
 * Note, you should use one instance of this per "context"(be it the class managing rendering)
 * This class is not thread safe. It is possible to access the same object from multiple threads if the same threadId
 * and DZConstPool object is used.
 */
public class DZConstPool {
    public class DZConstPoolInstance<T> {
        private final List<T> pool;
        private Supplier<T> create;

        public DZConstPoolInstance(int poolSize, Supplier<T> create) {
            this.create = create;
            List<T> tempPool = new ArrayList<>(poolSize);
            for (int i = 0; i < poolSize; i++) {
                tempPool.add(create.get());
            }
            this.pool = tempPool;
        }

        public T get(int threadId) {
            return pool.get(threadId);
        }
    }

    private final Map<Class<?>, DZConstPoolInstance<?>> poolInstances = new HashMap<>();

    // A unique object is returned per threadId, it must not exceed poolSize
    // +note, really annoying that we need Class<T> clazz in java :/
    public <T> T newObject(int poolSize, int threadId, Supplier<T> create, Class<T> clazz) {
        if (threadId >= poolSize) {
            throw new IllegalArgumentException("threadId must be between <= threadId < poolSize");
        }

        @SuppressWarnings("unchecked")
        DZConstPoolInstance<T> poolInstance = (DZConstPoolInstance<T>) poolInstances.computeIfAbsent(
            clazz, k -> new DZConstPoolInstance<>(poolSize, create)
        );

        return poolInstance.get(threadId);
    }
}