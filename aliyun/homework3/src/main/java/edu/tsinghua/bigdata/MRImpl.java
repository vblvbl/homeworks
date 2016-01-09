package edu.tsinghua.bigdata;

import com.csvreader.CsvReader;
import com.csvreader.CsvWriter;
import com.google.code.externalsorting.ExternalSort;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;

import com.aliyun.odps.mapred.Mapper;
import com.aliyun.odps.mapred.Reducer;
import com.aliyun.odps.mapred.WordCount;
import com.aliyun.odps.Column;
import com.aliyun.odps.data.ArrayRecord;
import com.aliyun.odps.OdpsType;

public class MRImpl implements Runnable {
    private Mapper map;
    private Reducer red;
    private final String inputFileName;
    private final String outputFileName;

    public MRImpl(String inputFileName, String outputFileName) {
        this.inputFileName = inputFileName;
        this.outputFileName = outputFileName;
    }

    public void SetMapRed(Mapper _map, Reducer _red){
        map = _map;
        red = _red;
    }

    @Override
    public void run() {
        try {
            long startTime = System.currentTimeMillis();
            //map
            Mapper mapper = new WordCount.TokenizerMapper();
            File inFile = new File(this.inputFileName);
            File mapResultFile = File.createTempFile("MapResult","csv");
            mapResultFile.deleteOnExit();
            CsvReader reader = new CsvReader(new FileReader(inFile));
            
            TaskContextImpl context = new TaskContextImpl();
            while (reader.readRecord()) {
                String[] values = reader.getValues();
                Long NbR = 0L;
                for (String w : values) {
                    mapper.setup(context);  
                    Column[] test = new Column[1];
                    test[0] = new Column("word", OdpsType.STRING);
                    ArrayRecord r = new ArrayRecord(test);
                    r.setString(0, w);
                    //System.out.println(r.getColumnCount());
                    //System.out.println(r.get(0));
                    mapper.map(NbR, r, context);
                    //writer.write(w);
                    //writer.write("1");
                    //writer.endRecord();
                    NbR = NbR + 1L;
                }
            }
            
            System.out.println(context.nb_records);
            // export to external file
            CsvWriter writer = new CsvWriter(new FileWriter(mapResultFile), ',');
            for(int i = 0; i < context.nb_records; ++i){
                RecordImpl key = context.keys.get(i);
                RecordImpl value = context.values.get(i);
                writer.write(key.get(0).toString());
                writer.write(value.get(0).toString());
                writer.endRecord();
            }
            writer.close();

            reader.close();

            //sort
            File sortedFile = File.createTempFile("SortedFile","csv");
            sortedFile.deleteOnExit();
            ExternalSort.sort(mapResultFile,sortedFile);

            //reduce
            File outputFile = new File(this.outputFileName);
            reader = new CsvReader(new FileReader(sortedFile));
            writer = new CsvWriter(new FileWriter(outputFile),',');
            String key = null;
            Long value = 0L;
            while(reader.readRecord()){
                String nextKey = reader.get(0);
                if(!nextKey.equals(key)){
                    if(key != null){
                        writer.write(key);
                        writer.write(value.toString());
                        writer.endRecord();
                    }
                    value  = 0L;
                    key = nextKey;
                }
                value += Long.parseLong(reader.get(1));
            }
            writer.write(key);
            writer.write(value.toString());
            writer.endRecord();

            reader.close();
            writer.close();

            System.out.println("Thread "+Thread.currentThread().getName()+" run time:"+(System.currentTimeMillis()-startTime));
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}