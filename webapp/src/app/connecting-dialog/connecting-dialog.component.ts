import { Component, OnInit, OnDestroy } from '@angular/core';
import { MatDialogRef } from '@angular/material/dialog';
import { Subscription } from 'rxjs';
import { ModelService } from '../model.service';

@Component({
  selector: 'app-connecting-dialog',
  templateUrl: './connecting-dialog.component.html',
  styleUrls: ['./connecting-dialog.component.css']
})
export class ConnectingDialogComponent implements OnInit, OnDestroy {

  connectedSubscription!: Subscription;

  constructor(
    private dialogRef: MatDialogRef<ConnectingDialogComponent>,
    private model: ModelService) {
      dialogRef.disableClose = true;
  }

  ngOnInit(): void {
    this.connectedSubscription = this.model.connectedSubject.asObservable().subscribe((b: boolean) => {this.close(b)});
  }

  ngOnDestroy(): void {
    this.connectedSubscription.unsubscribe();
  }

  close(b: boolean): void {
    if (b)
      this.dialogRef.close();
  }

}
