package com.afqa123.dukat;

import android.content.res.AssetManager;

public class DukatLib {

    // Used to load the 'native-lib' library on application startup.
    //static {
    //    System.loadLibrary("native-lib");
    //}

    public static native void init(AssetManager am);
}
