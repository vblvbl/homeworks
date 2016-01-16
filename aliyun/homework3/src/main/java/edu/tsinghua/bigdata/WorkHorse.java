package edu.tsinghua.bigdata;

import com.csvreader.CsvReader;
import com.csvreader.CsvWriter;
import com.google.code.externalsorting.ExternalSort;

import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.util.*;
import java.util.ArrayList;
import java.util.concurrent.Callable;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;

/**
 * Created by lx on 16/1/15.
 */
public class WorkHorse implements Callable<HashMap<String, Integer> > {
    private final String inputFileName;
    private final String outputFileName;

    public WorkHorse(String inputFileName) {
        this.inputFileName = inputFileName;
        outputFileName = new String("hello");
    }

    private void put_word(HashMap<String, Integer>map, String word) {
        if (map.containsKey(word)) {
            map.put(word, map.get(word)+1);
        } else {
            map.put(word, 1);
        }
    }

    @Override
    public HashMap<String, Integer> call() {
        
        try {
            File inFile = new File(this.inputFileName);
            CsvReader reader = new CsvReader(new FileReader(inFile));
            HashMap<String, Integer> hashMap = new HashMap<String, Integer>();

            while (reader.readRecord()) {
                String[] values = reader.getValues();
                for (String str : values) {
                    String[] words = str.split("\\s+");
                    for (String w : words) {
                        if(!w.isEmpty())

                            this.put_word(hashMap, w);
                    }
                }
            }

            reader.close();
            return hashMap;
        } catch (IOException e) {
            e.printStackTrace();
        }
        return new HashMap<String, Integer>();
    }
}
