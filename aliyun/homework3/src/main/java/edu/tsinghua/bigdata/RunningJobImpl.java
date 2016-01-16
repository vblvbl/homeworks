package edu.tsinghua.bigdata;

import com.aliyun.odps.counter.Counters;
import com.aliyun.odps.mapred.JobStatus;
import com.aliyun.odps.mapred.RunningJob;

import java.io.IOException;
import java.util.ArrayList;

import java.util.List;
import java.util.concurrent.Executors;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.ExecutionException;  
import java.util.concurrent.Future;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.Collections;
import java.util.Comparator;

import com.csvreader.CsvWriter;

import java.io.File;
import java.io.FileWriter;

/**
 * Created by zhangfei on 16-1-6.
 */
public class RunningJobImpl implements RunningJob{
    private String infile;
    private String outfile;
    private Thread task;
    List<String> files;
    public RunningJobImpl(String inputFileName, String outputFileName){
        infile = inputFileName;
        outfile = outputFileName;
    }

    public RunningJobImpl(List<String> string_list, String outputFileName){
        files = string_list;
        outfile = outputFileName;
    }

    public Thread getTask(){
        return this.task;
    }
    @Override
    public String getInstanceID() {
        return null;
    }

    @Override
    public boolean isComplete() {
        return false;
    }

    @Override
    public boolean isSuccessful() {
        return false;
    }

    @Override
    public void waitForCompletion() {
        ExecutorService exec = Executors.newFixedThreadPool(files.size()); 
        ArrayList<Future<HashMap<String, Integer> > > results = new ArrayList<Future<HashMap<String, Integer> > >(); 
        
        exec.submit(new WorkHorse(files.get(0)));
        for(String file : files){
            results.add(exec.submit(new WorkHorse(file)));
        } 
        exec.shutdown();

        HashMap<String, Integer> core = new HashMap<String, Integer>();
        try{
            core = results.get(0).get();
            results.remove(0);
        } catch (InterruptedException e) {  
            e.printStackTrace();  
        } catch (ExecutionException e) {  
            e.printStackTrace();  
        }  

        for(Future<HashMap<String, Integer> > fs : results){  
            try {
                HashMap<String, Integer> hm = fs.get();
                if( hm.isEmpty() ) continue;
                Iterator<String> it = hm.keySet().iterator();
                while(it.hasNext()) {
                    String key = (String)it.next();
                    Integer v = hm.get(key);
                    if(!core.containsKey(key)){
                        core.put(key, v );
                    } else {
                        core.put(key, core.get(key) + v);
                    }
                }
            } catch (InterruptedException e) {  
                e.printStackTrace();  
            } catch (ExecutionException e) {  
                e.printStackTrace();  
            }  
        }

        List<Map.Entry<String, Integer>> sortedMap =
                new ArrayList<Map.Entry<String, Integer>>(core.entrySet());

        Collections.sort(sortedMap, new Comparator<Map.Entry<String, Integer>>() {
            @Override
            public int compare(Map.Entry<String, Integer> o1, Map.Entry<String, Integer> o2) {
                return o1.getKey().compareTo(o2.getKey());
            }
        });

        try{
            String blank = new String(" ");
            File outputFile = new File(this.outfile);
            CsvWriter writer = new CsvWriter(new FileWriter(outputFile), ',');
            for(Map.Entry<String, Integer> entry : sortedMap) {
                    writer.write(entry.getKey());
                    writer.write(entry.getValue().toString());
                    writer.endRecord();
            }
            writer.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
        
        
    }

    @Override
    public JobStatus getJobStatus() {
        return null;
    }

    @Override
    public void killJob() {

    }

    @Override
    public Counters getCounters() {
        return null;
    }

    @Override
    public String getDiagnostics() {
        return null;
    }

    @Override
    public float mapProgress() throws IOException {
        return 0;
    }

    @Override
    public float reduceProgress() throws IOException {
        return 0;
    }
}
