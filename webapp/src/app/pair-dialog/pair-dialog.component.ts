import { Component, OnInit } from '@angular/core';
import { MatDialogRef } from '@angular/material/dialog';
import { Subscription } from 'rxjs';
import { ModelService } from '../model.service';

@Component({
  selector: 'app-pair-dialog',
  templateUrl: './pair-dialog.component.html',
  styleUrls: ['./pair-dialog.component.css']
})
export class PairDialogComponent implements OnInit {

  mode: string = '';

  constructor(
    private dialogRef: MatDialogRef<PairDialogComponent>,
    public model: ModelService) {
      dialogRef.disableClose = true;
      this.mode = model.mode.value;
  }

  ngOnInit(): void {
    this.scan();
  }

  scan(): void {
    this.model.scanBTDevices().subscribe({
      next: (res: string) => {
        console.info('scanBTDevices: ' + res);
      }
    });
  }

  pair(address: string): void {
    this.model.pairBTDevice(address).subscribe({
      next: (res: string) => {
        console.info('pairBTDevice: ' + res);
        if (res == 'OK')
          this.dialogRef.close(true);
      }
    });
  }

  cancel(): void {
    this.dialogRef.close(false);
  }

}
