import { Component, HostListener } from '@angular/core';
import { MatDialogRef } from '@angular/material/dialog';
import { FormControl, FormGroup } from '@angular/forms';
import { ModelService } from '../model.service';

@Component({
  selector: 'app-settings-dialog',
  templateUrl: './settings-dialog.component.html',
  styleUrls: ['./settings-dialog.component.css']
})
export class SettingsDialogComponent {

  form = new FormGroup({
    mode: new FormControl(''),
    wifiMode: new FormControl(''),
    name: new FormControl(''),
    houseSSID: new FormControl(''),
    housePassword: new FormControl(''),
    tractorSSID: new FormControl(''),
    tractorPassword: new FormControl(''),
    tractorAddress: new FormControl('')
  });

  showHousePassword = false;
  showTractorPassword = false;

  constructor(
    private dialogRef: MatDialogRef<SettingsDialogComponent>,
    private model: ModelService
    ) {
      dialogRef.disableClose = true;
      this.form.setValue({
        mode: model.mode,
        wifiMode: model.wifiMode,
        name: model.name,
        houseSSID: model.houseSSID,
        housePassword: model.housePassword,
        tractorSSID: model.tractorSSID,
        tractorPassword: model.tractorPassword,
        tractorAddress: model.tractorAddress
    });
  }

  @HostListener('window:keyup.Enter', ['$event'])
  ok(): void {
    this.model.io.emit('configure', this.form.value, (res: any) => {
      console.log('configure:', res);
      if (res)
        this.dialogRef.close(true);
    });
  }

  @HostListener('window:keyup.Esc', ['$event'])
  cancel(): void {
    this.dialogRef.close(false);
  }

}
