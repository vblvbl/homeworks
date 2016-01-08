package com.aliyun.odps.mapred;

import com.aliyun.odps.OdpsException;
import com.aliyun.odps.conf.Configured;

public class JobRunnerImpl extends Configured implements JobRunner {
  @Override public RunningJob submit() throws OdpsException {
    RunningJobImpl rnj = new RunningJobImpl();
    rnj.setConf(this.getConf());
    return rnj;
  }
}
