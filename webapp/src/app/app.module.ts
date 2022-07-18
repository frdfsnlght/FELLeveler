import { NgModule } from '@angular/core';
import { BrowserModule, Title } from '@angular/platform-browser';

import { AppComponent } from './app.component';
import { BrowserAnimationsModule } from '@angular/platform-browser/animations';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';
import { HttpClientModule } from '@angular/common/http';
import { MaterialModule } from './material/material.module';
import { SettingsDialogComponent } from './settings-dialog/settings-dialog.component';
import { RebootDialogComponent } from './reboot-dialog/reboot-dialog.component';
import { CalibrateDialogComponent } from './calibrate-dialog/calibrate-dialog.component';
import { ConnectingDialogComponent } from './connecting-dialog/connecting-dialog.component';

@NgModule({
  declarations: [
    AppComponent,
    SettingsDialogComponent,
    RebootDialogComponent,
    CalibrateDialogComponent,
    ConnectingDialogComponent
  ],
  entryComponents: [
    SettingsDialogComponent
  ],
  imports: [
    BrowserModule,
    BrowserAnimationsModule,
    FormsModule,
    ReactiveFormsModule,
    HttpClientModule,
    MaterialModule
  ],
  providers: [
    Title
  ],
  bootstrap: [AppComponent]
})
export class AppModule { }
