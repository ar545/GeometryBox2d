package edu.cornell.gdiac.geometry;

import org.libsdl.app.*;
import android.os.Bundle;

public class Geometry extends SDLActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        // Make sure this is before calling super.onCreate
        setTheme(R.style.CUGLTheme);
        super.onCreate(savedInstanceState);
    }

}
