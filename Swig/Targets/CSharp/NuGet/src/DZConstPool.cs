using System;
using System.Collections.Generic;

namespace DenOfIz
{
    /**
     * A very simple pool implementation, this is simply for demonstration purposes when the amount of objects
     * you will use are known beforehand, even better use static classes, but this is here if you'd like to keep LOC short.
     * Note, you should use one instance of this per "context"(be it the class managing rendering)
     * This class is not thread safe. It is possible to access the same object from multiple threads if the same threadId
     * and DZConstPool object is used.
     */
    public class DZConstPool
    {
        public class DZConstPoolInstance<T>
        {
            private readonly List<T> pool;
            public Func<T> Create { get; set; }

            public DZConstPoolInstance(int poolSize, Func<T> create)
            {
                Create = create;
                pool = new List<T>(poolSize);
                for (int i = 0; i < poolSize; i++)
                {
                    pool.Add(create());
                }
            }

            public T Get(int threadId)
            {
                return pool[threadId];
            }
        }

        private readonly Dictionary<Type, object> poolInstances = new Dictionary<Type, object>();

        // A unique object is returned per threadId, it must not exceed poolSize
        public T NewObject<T>(int poolSize, int threadId, Func<T> create)
        {
            if (threadId >= poolSize)
            {
                throw new ArgumentException("threadId must be between <= threadId < poolSize");
            }

            Type type = typeof(T);
            if (!poolInstances.ContainsKey(type))
            {
                poolInstances[type] = new DZConstPoolInstance<T>(poolSize, create);
            }

            DZConstPoolInstance<T> poolInstance = (DZConstPoolInstance<T>)poolInstances[type];
            return poolInstance.Get(threadId);
        }
    }
}