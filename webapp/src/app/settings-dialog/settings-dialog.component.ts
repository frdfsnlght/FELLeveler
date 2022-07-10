import { Component, HostListener, OnInit } from '@angular/core';
import { MatDialogRef } from '@angular/material/dialog';
import { FormControl, FormGroup } from '@angular/forms';
import { ModelService } from '../model.service';

@Component({
  selector: 'app-settings-dialog',
  templateUrl: './settings-dialog.component.html',
  styleUrls: ['./settings-dialog.component.css']
})
export class SettingsDialogComponent implements OnInit {

  form = new FormGroup({
    mode: new FormControl(''),
    name: new FormControl(''),
    wifiSSID: new FormControl(''),
    wifiPassword: new FormControl('')
  });

  showPassword = false;

  constructor(
    private dialogRef: MatDialogRef<SettingsDialogComponent>,
    private model: ModelService
    ) {
      dialogRef.disableClose = true;
      this.form.setValue({
        mode: model.mode.value,
        name: model.name.value,
        wifiSSID: model.wifiSSID.value,
        wifiPassword: model.wifiPassword.value
    });
  }

  ngOnInit(): void {
  }

  @HostListener('window:keyup.Enter', ['$event'])
  ok(): void {
    this.model.configure(this.form.value).subscribe({
      next: (res: string) => {
        console.info('configure: ' + res);
        if (res == 'OK')
          this.dialogRef.close(true);
      }
    });
  }

  @HostListener('window:keyup.Esc', ['$event'])
  cancel(): void {
    this.dialogRef.close(false);
  }

}
