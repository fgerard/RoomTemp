(require '[clojure.java.io :as io] 
         '[clojure.string :as S])

(def values 
  (reduce 
   (fn [result line]
     (let [vals (S/split (S/trim line) #" +")]
       (println (pr-str vals))
       (conj result (vec (map #(Double/parseDouble %) vals)))))
   []
   (line-seq (io/reader "values.txt")))) values

(with-open [out (io/writer (io/file "init_values.h"))]
  (.write out "#define INIT_DATA_TMP")
  (let [[temp1 _ _] (first values)]
    (.write out (format "  %.2f" temp1)))
  (doseq [[temp _ _] (rest values)]
    (.write out (format ",%.2f" temp)))
  (.write out "\n\n")
  (.write out "#define INIT_DATA_HUMIDITY")
  (let [[_ hum1 _] (first values)]
    (.write out (format "  %.2f" hum1)))
  (doseq [[_ hum _] (rest values)]
    (.write out (format ",%.2f" hum)))
  (.write out "\n\n")

  (.write out "#define INIT_DATA_PRESSURE")
  (let [[_ _ pres1] (first values)]
    (.write out (format "  %.2f" pres1)))
  (doseq [[_ _ pres] (rest values)]
    (.write out (format ",%.2f" pres)))
  (.write out "\n\n"))
