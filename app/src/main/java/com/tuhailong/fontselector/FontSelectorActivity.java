package com.tuhailong.fontselector;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.util.Log;
import android.util.Pair;
import android.widget.TextView;

import java.io.File;

public class FontSelectorActivity extends AppCompatActivity {
    private static final String TAG = "FontSelector";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        TextView textView = findViewById(R.id.edit_text);
        Pair<File, Integer> fontForA = SystemFontHelper.matchFamilyStyleCharacter(
                "sans-serif", 400, false, "zh-Hans", 0 /* default family variant */, "ABCxyz");
        Pair<File, Integer> fontForB = SystemFontHelper.matchFamilyStyleCharacter(
                "sans-serif", 400, false, "zh-Hans", 0 /* default family variant */, "123!@#");
        String msg = "zh-Hans: " + fontForA + " for ABCxyz";
        msg += "\nzh-Hans: " + fontForB + " for 123!@#";

        fontForA = SystemFontHelper.matchFamilyStyleCharacter(
                "sans-serif", 400, false, "zh-Hans", 0 /* default family variant */, "茕茕孑立");
        fontForB = SystemFontHelper.matchFamilyStyleCharacter(
                "sans-serif", 400, false, "zh-Hans", 0 /* default family variant */, "魑魅魍魉");
        msg += "\nzh-Hans: " + fontForA + " for 茕茕孑立";
        msg += "\nzh-Hans: " + fontForB + " for 魑魅魍魉";

        textView.setText(msg);
        Log.i(TAG, msg);
    }
}
