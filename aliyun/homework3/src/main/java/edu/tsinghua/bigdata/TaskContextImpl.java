package edu.tsinghua.bigdata;

import com.aliyun.odps.mapred.Mapper;
import com.aliyun.odps.mapred.Reducer;
import com.aliyun.odps.mapred.Mapper.TaskContext;
import com.aliyun.odps.data.Record;
import com.aliyun.odps.data.TableInfo;
import com.aliyun.odps.volume.FileSystem;
import com.aliyun.odps.data.VolumeInfo;
import com.aliyun.odps.Column;
import com.aliyun.odps.mapred.conf.JobConf;

import java.io.IOException;
import java.io.BufferedInputStream;
import com.aliyun.odps.counter.Counter;
import java.util.Iterator;
import com.aliyun.odps.mapred.TaskId;

public class TaskContextImpl implements Mapper.TaskContext {
	@Override
	public long getCurrentRecordNum(){
		return 1;
	}

	@Override
	public Record getCurrentRecord(){
		return null;
	}

	@Override
	public boolean nextRecord(){
		return true;
	}

	@Override
	public TableInfo[] getOutputTableInfo(){
		return null;
	}

	@Override
	public TableInfo getInputTableInfo(){
		return null;
	}

	@Override
	public Record createOutputRecord() throws IOException{
		return null;
	}

	@Override
	public Record createOutputRecord(String label) throws IOException{
		return null;
	}

	@Override
	public Record createOutputKeyRecord() throws IOException{
		return null;
	}

	@Override
	public Record createOutputValueRecord() throws IOException{
		return null;
	}

	@Override
	public Record createMapOutputKeyRecord() throws IOException{
		return null;
	}

	@Override
	public Record createMapOutputValueRecord() throws IOException{
		return null;
	}

	@Override
	public BufferedInputStream readResourceFileAsStream(String resourceName) throws IOException{
		return null;
	}

	@Override
	public Iterator<Record> readResourceTable(String resourceName) throws IOException{
		return null;
	}

	@Override
	public Counter getCounter(Enum<?> name){
		return null;
	}

	@Override
	public Counter getCounter(String group,
                   String name){
		return null;
	}

	@Override
	public TaskId getTaskID() {
		return null;
	}

	@Override
	public void progress(){
	}

	@Override
	public void write(Record record) throws IOException{
    }

    @Override
    public void write(Record record,
           String label) throws IOException{
    }

    @Override
    public void write(Record key,
           Record value) throws IOException{
    }

    @Override
    public FileSystem getOutputVolumeFileSystem(String a){
    	return null;
    }

    @Override
    public FileSystem getOutputVolumeFileSystem(){
    	return null;
    }

    @Override
    public FileSystem getInputVolumeFileSystem(String a){
    	return null;
    }

    @Override
    public FileSystem getInputVolumeFileSystem(){
    	return null;
    }

    @Override
    public VolumeInfo getOutputVolumeInfo(String a){
    	return null;
    }

    @Override
    public VolumeInfo getOutputVolumeInfo(){
    	return null;
    }

    @Override
    public VolumeInfo getInputVolumeInfo(String a){
    	return null;
    }

    @Override
    public VolumeInfo getInputVolumeInfo(){
    	return null;
    }

    @Override
    public String[] getGroupingColumns(){
    	return null;
    }

    @Override
    public  Class<? extends Reducer> getReducerClass(){
    	return null;
    }

    @Override
    public  Class<? extends Reducer> getCombinerClass(){
    	return null;
    }

    @Override
    public  Class<? extends Mapper> getMapperClass(){
    	return null;
    }

    @Override
    public Column[] getMapOutputValueSchema(){
    	return null;
    }

    @Override
    public Column[] getMapOutputKeySchema(){
    	return null;
    }

    @Override
    public int getNumReduceTasks(){
    	return 1;
    }

    @Override
    public JobConf getJobConf(){
    	return null;
    }
}