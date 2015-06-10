/*
 * Licensed to the University of California, Berkeley under one or more contributor license
 * agreements. See the NOTICE file distributed with this work for additional information regarding
 * copyright ownership. The ASF licenses this file to You under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the License. You may obtain a
 * copy of the License at
 * 
 * http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software distributed under the License
 * is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express
 * or implied. See the License for the specific language governing permissions and limitations under
 * the License.
 */

package tachyon.client;

import java.io.IOException;

import tachyon.TachyonURI;

import tachyon.client.OutStream;
import tachyon.client.InStream;
import tachyon.client.TachyonFile;
import tachyon.client.TachyonFS;
import tachyon.client.WriteType;
import tachyon.client.ReadType;

public class TachyonKV {
  private static final String KVSTORE_PREFIX = "/jkvstore/";

  private TachyonFS mTachyonClient;
  private String mKVStore;
  private ReadType mReadType;
  private WriteType mWriteType;
  private long mBlockSizeByte;

  public TachyonKV(TachyonFS client, ReadType readType, WriteType writeType, long blockSizeByte) {
    this(client, readType, writeType, blockSizeByte, KVSTORE_PREFIX);
  }

  public TachyonKV(TachyonFS client, ReadType readType, WriteType writeType, 
        long blockSizeByte, String kvStore) {
    mTachyonClient = client;
    // must ends with '/'
    if (kvStore.charAt(kvStore.length() - 1) != '/') {
      mKVStore = kvStore + '/';
    } else {
      mKVStore = kvStore;
    }
    mReadType = readType;
    mWriteType = writeType;
    mBlockSizeByte = blockSizeByte;
  }

  public boolean init() {
    try {
      return mTachyonClient.mkdir(new TachyonURI(mKVStore));
    } catch (IOException e) {
      return false;
    }
  }

  public void write(String key, byte[] value) throws IOException {
    String filePath = mKVStore + key;
    TachyonFile file = mTachyonClient.getFile(filePath);
    if (file != null) {
      if (!mTachyonClient.delete(filePath, false)) {
        System.out.println("fail to delete existing " + filePath);
      }
    }
    int fid = mTachyonClient.createFile(new TachyonURI(filePath), mBlockSizeByte);
    if (fid > 0) {
      file = mTachyonClient.getFile(fid);
    }
    if (file != null) {
      OutStream os = file.getOutStream(mWriteType);
      if (os != null) {
        os.write(value);
        os.close();
      }
    }
  }

  public int read(String key, byte[] value) throws IOException {
    int sz = -1;
    String filePath = mKVStore + key;
    TachyonFile file = mTachyonClient.getFile(filePath);
    if (file == null) {
      System.out.println("fail to get file " + filePath);
      return sz;
    }
    InStream is = file.getInStream(mReadType);
    if (is != null) {
      sz = is.read(value);
      System.out.println("read " + sz);
      is.close();
    }
    return sz;
  }
}
